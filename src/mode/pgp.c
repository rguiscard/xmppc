/*!
 * @file pgp.c
 *
 * vim: expandtab:ts=2:sts=2:sw=2
 * 
 * @authors
 * Copyright (C) 2020 Anoxinon e.V.
 *
 * @copyright 
 * This file is part of xmppc.
 *
 * xmppc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xmppc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * German
 *
 * Diese Datei ist Teil von xmppc.
 *
 * xmppc ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * xmppc wird in der Hoffnung, dass es nützlich sein wird, aber
 * OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
 * Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Details.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#include "pgp.h"
#include "string.h"

#include <locale.h>
#include <stdlib.h>
#include <gpgme.h>

#define PGP_BEGIN "-----BEGIN PGP MESSAGE-----"
#define PGP_END   "-----END PGP MESSAGE-----"

static char* _pgp_encrypt_message(xmppc_t *xmppc, char* recipient, char* message);
static char* _pgp_remove_PGP_MESSAGE_comment(const char* message);

static void _pgp_send_text(xmppc_t *xmppc, char* to, char* text);

void pgp_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if(argc > 0) {
    if(strcmp("chat", argv[0]) == 0) {
      _pgp_send_text(xmppc, argv[1], argv[2]);
    } else {
      logError(xmppc, "Unbekanner Befehl: %s\n", argv[0]);
    }
  }
  sleep(10);
  xmpp_disconnect(xmppc->conn);
}

void _pgp_send_text(xmppc_t *xmppc, char* to, char* text) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *message;
  char* id = xmpp_uuid_gen(xmppc->ctx);
  message = xmpp_message_new(xmpp_conn_get_context(conn), "chat", to, id);
  int res = xmpp_message_set_body(message, "This message is *encrypted* with PGP (See :XEP:`27`)");
  if(res == 0) {
    xmpp_stanza_t *x = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(x, "x");
    xmpp_stanza_set_ns(x, "jabber:x:encrypted");
    xmpp_stanza_t *b = xmpp_stanza_new(xmppc->ctx);
    char* encrypt_text = _pgp_encrypt_message(xmppc, to,text);
    if(encrypt_text == NULL) {
      logError(xmppc,"Encrypting of message failed.\n");
      return;
    }
    xmpp_stanza_set_text(b,encrypt_text);
    xmpp_stanza_add_child(x, b);
    xmpp_stanza_add_child(message, x);
    xmpp_stanza_t *e = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(e, "encryption");
    xmpp_stanza_set_ns(e, "urn:xmpp:eme:0");
    xmpp_stanza_set_attribute(e, "namespace", "jabber:x:encrypted");
    xmpp_stanza_add_child(message, e);
    xmpp_send(conn, message); 
  }
}

char* _pgp_encrypt_message(xmppc_t *xmppc, char* recipient, char* message) {
  setlocale (LC_ALL, "");
  gpgme_check_version (NULL);
  gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
  gpgme_ctx_t ctx;
  gpgme_error_t error = gpgme_new (&ctx);
  if(GPG_ERR_NO_ERROR != error ) {
    printf("gpgme_new: %d\n", error);
    return NULL;
  }
  error = gpgme_set_protocol(ctx, GPGME_PROTOCOL_OPENPGP);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }
  gpgme_set_armor(ctx,1);
  gpgme_set_textmode(ctx,1);
  gpgme_set_offline(ctx,1);
  gpgme_set_keylist_mode(ctx, GPGME_KEYLIST_MODE_LOCAL);

  gpgme_key_t recp[3];

  // Key for sender
  const char *jid = xmpp_conn_get_jid(xmppc->conn);
  logInfo(xmppc, "Looking up pgp key for %s\n", jid);
  error = gpgme_get_key(ctx, jid, &(recp[0]), 0);
  if(error != 0) {
    logError(xmppc,"Public key not found for %s. GpgME Error: %s\n", jid, gpgme_strerror(error));
    return NULL;
  }

  // Key for recipient
  logInfo(xmppc, "Looking up pgp key for %s\n", recipient);
  error = gpgme_get_key(ctx, recipient, &(recp[1]), 0);
  if(error != 0) {
    logError(xmppc,"Key not found for %s. GpgME Error: %s\n", recipient, gpgme_strerror(error));
    return NULL;
  }
  recp[2] = NULL;
  
  logInfo(xmppc, "%s <%s>\n", recp[0]->uids->name, recp[0]->uids->email);
  logInfo(xmppc, "%s <%s>\n", recp[1]->uids->name, recp[1]->uids->email);

  gpgme_encrypt_flags_t flags = 0;
  gpgme_data_t plain;
  gpgme_data_t cipher;

  error = gpgme_data_new (&plain);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }

  error = gpgme_data_new_from_mem(&plain, message, strlen(message),0);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }
  error = gpgme_data_new (&cipher);  
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }

  error = gpgme_op_encrypt ( ctx, recp, flags, plain, cipher);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }
  size_t len;
  char *cipher_str = gpgme_data_release_and_get_mem(cipher, &len);
  char* result = NULL;
  if(len > 0 ){
    result = _pgp_remove_PGP_MESSAGE_comment(cipher_str);
  }

  gpgme_key_release (recp[0]);
  gpgme_key_release (recp[1]);
  gpgme_release (ctx);
  return result;
}

static char* _pgp_remove_PGP_MESSAGE_comment(const char* message) {
  char* tmp = alloca((strlen(message) - strlen(PGP_END)) + 1);
  strncpy(tmp, message, strlen(message) - strlen(PGP_END));
  tmp[(strlen(message) - strlen(PGP_END)+1)] = '\0';
  tmp = tmp+((strlen(PGP_BEGIN) +1) * sizeof(char));
  char* result = malloc(strlen(tmp)+1);
  strcpy(result, tmp);
  return result;
}
