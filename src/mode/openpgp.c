/*!
 * @file openpgp.c
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

#include "openpgp.h"
#include "string.h"

#include <locale.h>
#include <stdlib.h>
#include <glib.h>
#include <gpgme.h>

static void _openpgp_send_text(xmppc_t *xmppc, char* to, char* text);
static xmpp_stanza_t* _openpgp_signcrypt(xmppc_t *xmppc, char* to, char* text);
static char* _openpgp_gpg_signcrypt(xmppc_t *xmppc, char* recipient, char* message); 
static gpgme_error_t _openpgp_lookup_key(xmppc_t *xmppc, char* name, gpgme_ctx_t* ctx, gpgme_key_t* key);

void openpgp_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if(argc > 0) {
    if(strcmp("signcrypt", argv[0]) == 0) {
      _openpgp_send_text(xmppc, argv[1], argv[2]);
    } else {
      logError(xmppc, "Unbekanner Befehl: %s\n", argv[0]);
    }
  }
  xmpp_disconnect(xmppc->conn);
}

void _openpgp_send_text(xmppc_t *xmppc, char* to, char* text) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *message;
  char* id = xmpp_uuid_gen(xmppc->ctx);
  message = xmpp_message_new(xmpp_conn_get_context(conn), NULL, to, id);
  xmpp_message_set_body(message, "This message is *encrypted* with OpenPGP (See :XEP:`0373`)");
    xmpp_stanza_t *openpgp = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(openpgp, "openpgp");
    xmpp_stanza_set_ns(openpgp, "urn:xmpp:openpgp:0");
    
    xmpp_stanza_t * signcrypt = _openpgp_signcrypt(xmppc, to, text);
    char* c;
    size_t s;
    xmpp_stanza_to_text(signcrypt, &c,&s);
    char* signcrypt_e = _openpgp_gpg_signcrypt(xmppc,to, c);
    if( signcrypt_e == NULL ) {
      logError(xmppc, "Message not signcrypted.\n");
      return;
    } 
    // BASE64_OPENPGP_MESSAGE
    xmpp_stanza_t* base64_openpgp_message = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_text(base64_openpgp_message,signcrypt_e);
    xmpp_stanza_add_child(openpgp, base64_openpgp_message);
    xmpp_stanza_add_child(message, openpgp);
    
    xmpp_stanza_to_text(message, &c,&s);
    
    xmpp_send(conn, message); 
   
}

xmpp_stanza_t* _openpgp_signcrypt(xmppc_t *xmppc, char* to, char* text) {

  time_t now = time(NULL);
  struct tm* tm = localtime(&now);
  char buf[255];
  strftime(buf, sizeof(buf), "%FT%T%z", tm);
    int randnr = rand() % 5;
    char rpad_data[randnr];
    for(int i = 0; i < randnr-1; i++) {
      rpad_data[i] = 'c';
    }
    rpad_data[randnr-1] = '\0';
    
    // signcrypt
    xmpp_stanza_t *signcrypt = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(signcrypt, "signcrypt");
    xmpp_stanza_set_ns(signcrypt, "urn:xmpp:openpgp:0");
    // to
    xmpp_stanza_t *s_to = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(s_to, "to");
    xmpp_stanza_set_attribute(s_to, "jid", to);
    // time
    xmpp_stanza_t *time = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(time, "time");
    xmpp_stanza_set_attribute(time, "stamp", buf);
    xmpp_stanza_set_name(time, "time");
    // rpad
    xmpp_stanza_t *rpad = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(rpad, "rpad");
    xmpp_stanza_t *rpad_text = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_text(rpad_text, rpad_data);
    // payload
    xmpp_stanza_t *payload= xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(payload, "payload");
    // body
    xmpp_stanza_t *body = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(body, "body");
    xmpp_stanza_set_ns(body, "jabber:client");
    // text
    xmpp_stanza_t *body_text = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_text(body_text, text);
    xmpp_stanza_add_child(signcrypt,s_to);
    xmpp_stanza_add_child(signcrypt,time);
    xmpp_stanza_add_child(signcrypt,rpad);
    xmpp_stanza_add_child(rpad,rpad_text);
    xmpp_stanza_add_child(signcrypt,payload);
    xmpp_stanza_add_child(payload, body);
    xmpp_stanza_add_child(body, body_text);

    return signcrypt;
}

char* _openpgp_gpg_signcrypt(xmppc_t *xmppc, char* recipient, char* message) {
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
  gpgme_set_armor(ctx,0);
  gpgme_set_textmode(ctx,0);
  gpgme_set_offline(ctx,1);
  gpgme_set_keylist_mode(ctx, GPGME_KEYLIST_MODE_LOCAL);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
  }

  gpgme_key_t recp[3];
  recp[0] = NULL,
  recp[1] = NULL;
  const char *jid = xmpp_conn_get_jid(xmppc->conn);
  
  char* xmpp_jid_me = alloca( (strlen(jid)+6) * sizeof(char) );
  char* xmpp_jid_recipient =  alloca( (strlen(recipient)+6) * sizeof(char) );

  strcpy(xmpp_jid_me, "xmpp:");
  strcpy(xmpp_jid_recipient, "xmpp:");
  strcat(xmpp_jid_me, jid);
  strcat(xmpp_jid_recipient,recipient);

  gpgme_signers_clear(ctx);

  // lookup own key
  error = _openpgp_lookup_key(xmppc,xmpp_jid_me, &ctx, &recp[0]);
  if(error != 0) {
    logError(xmppc,"Key not found for %s. GpgME Error: %s\n", xmpp_jid_me, gpgme_strerror(error));
    return NULL;
  }

  error = gpgme_signers_add(ctx,recp[0]);
  if(error != 0) {
    logError(xmppc,"gpgme_signers_add %s. GpgME Error: %s\n", xmpp_jid_me, gpgme_strerror(error));
    return NULL;
  }


  // lookup key of recipient
  error = _openpgp_lookup_key(xmppc,xmpp_jid_recipient, &ctx, &recp[1]);
  if(error != 0) {
    logError(xmppc,"Key not found for %s. GpgME Error: %s\n", xmpp_jid_recipient, gpgme_strerror(error));
    return NULL;
  }
  recp[2] = NULL;
  logInfo(xmppc, "%s <%s>\n", recp[0]->uids->name, recp[0]->uids->email);
  logInfo(xmppc, "%s <%s>\n", recp[1]->uids->name, recp[1]->uids->email);

#ifdef XMPPC_DEVELOPMENT
  gpgme_encrypt_flags_t flags = GPGME_ENCRYPT_ALWAYS_TRUST; 
#else 
  gpgme_encrypt_flags_t flags = 0;
#endif

  gpgme_data_t plain;
  gpgme_data_t cipher;

  error = gpgme_data_new (&plain);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
    return NULL;
  }

  error = gpgme_data_new_from_mem(&plain, message, strlen(message),0);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
    return NULL;
  }
  error = gpgme_data_new (&cipher);  
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
    return NULL;
  }

  error = gpgme_op_encrypt_sign ( ctx, recp, flags, plain, cipher);
  if(error != 0) {
    logError(xmppc,"GpgME Error: %s\n", gpgme_strerror(error));  
    return NULL;
  }

  size_t len;
  char *cipher_str = gpgme_data_release_and_get_mem(cipher, &len);
  char* result = g_base64_encode( (unsigned char*) cipher_str,len);
  gpgme_key_release (recp[0]);
  gpgme_key_release (recp[1]);
  gpgme_release (ctx);
  return result;
}

gpgme_error_t _openpgp_lookup_key(xmppc_t *xmppc,char* name, gpgme_ctx_t* ctx, gpgme_key_t* key) {
  logDebug(xmppc, "Looking for key: %s ...\n", name);
  gpgme_error_t error = gpgme_op_keylist_start (*ctx, NULL, 0);
  while (!error) {
    error = gpgme_op_keylist_next (*ctx, key);
    if(!error) {
      gpgme_user_id_t uids = (*key)->uids;
      while (uids) {
        if(strcmp(uids->name, name) == 0) {
         logDebug(xmppc, "Key found: %s ...\n", uids->name);
         return error; 
        } 
        uids=uids->next;
      }
    } else {
      gpgme_key_release((*key));
    }
  }
  return error;
}

