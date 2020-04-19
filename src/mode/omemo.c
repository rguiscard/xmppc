/*!
 * @file omemo.c
 *
 * vim: expandtab:ts=2:sts=2:sw=2
 *
 * @copyright
 *
 * Copyright (C) 2020 Anoxinon e.V.
 *
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

#include "omemo.h"

#include <glib.h>
#include <unistd.h>
#define DJB_TYPE 0x05

static int response = 0;

static void _omemo_device_list_query(xmppc_t *xmppc);
static int _omemo_device_list_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata);

static void _omemo_bundles_query(xmppc_t *xmppc, const char* deviceid);
static int _omemo_bundles_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata);

void omemo_execute_command(xmppc_t *xmppc, int agrc, char *argv[]) {
  _omemo_device_list_query(xmppc);
}

void _omemo_device_list_query(xmppc_t *xmppc) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *iq, *query, *item;
  char* id = xmpp_uuid_gen(xmppc->ctx);
  iq = xmpp_iq_new(xmpp_conn_get_context(conn), "get", id);
  const char *jid = xmpp_conn_get_jid(conn);
  xmpp_stanza_set_from(iq, jid);
  xmpp_stanza_set_to(iq, jid);
  query = xmpp_stanza_new(xmpp_conn_get_context(conn));
  xmpp_stanza_set_name(query, "pubsub");
  xmpp_stanza_set_ns(query, "http://jabber.org/protocol/pubsub");
  xmpp_stanza_add_child(iq, query);
  item = xmpp_stanza_new(xmpp_conn_get_context(conn));
  xmpp_stanza_set_name(item, "items");
  xmpp_stanza_set_attribute(item, "node",
                            "eu.siacs.conversations.axolotl.devicelist");
  xmpp_stanza_add_child(query, item);
  xmpp_stanza_release(query);
  xmpp_stanza_release(item);
  xmpp_id_handler_add(conn, _omemo_device_list_reply , id, xmppc);
  xmpp_send(conn, iq);
}

static int _omemo_device_list_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata) {
  xmppc_t *xmppc = (xmppc_t *)userdata;
  xmpp_stanza_t *query, *item;
  const char *name;
  query = xmpp_stanza_get_child_by_name(stanza, "pubsub");
  query = xmpp_stanza_get_child_by_name(query, "items");
  query = xmpp_stanza_get_child_by_name(query, "item");
  query = xmpp_stanza_get_child_by_name(query, "list");
  for (item = xmpp_stanza_get_children(query); item;
       item = xmpp_stanza_get_next(item))
    if ((name = xmpp_stanza_get_attribute(item, "id"))) {
      response++;
      logInfo(xmppc,"\t %s\n", name);
      _omemo_bundles_query(xmppc, name);
    }
    
  return 0;
}

void _omemo_bundles_query(xmppc_t *xmppc, const char* deviceid){
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *iq, *query, *item;
//  char* id = xmpp_uuid_gen(xmppc->ctx);
  iq = xmpp_iq_new(xmpp_conn_get_context(conn), "get", deviceid);
  const char *jid = xmpp_conn_get_jid(conn);
  xmpp_stanza_set_from(iq, jid);
  xmpp_stanza_set_to(iq, jid);
  query = xmpp_stanza_new(xmpp_conn_get_context(conn));
  xmpp_stanza_set_name(query, "pubsub");
  xmpp_stanza_set_ns(query, "http://jabber.org/protocol/pubsub");
  xmpp_stanza_add_child(iq, query);
  item = xmpp_stanza_new(xmpp_conn_get_context(conn));
  xmpp_stanza_set_name(item, "items");
  char bundle[100] = "";
  strcat(bundle, "eu.siacs.conversations.axolotl.bundles:");
  strcat(bundle, deviceid);
  xmpp_stanza_set_attribute(item, "node",bundle);
  xmpp_stanza_add_child(query, item);
  xmpp_stanza_release(query);
  xmpp_stanza_release(item);
  xmpp_id_handler_add(conn, _omemo_bundles_reply , deviceid, xmppc);
  xmpp_send(conn, iq);

}

int _omemo_bundles_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata)  {
  xmppc_t *xmppc = (xmppc_t *)userdata;

  if(strcmp(xmpp_stanza_get_type(stanza), "error") == 0) {
    printf("Fehler\n");
    return 0;
  } 

  xmpp_stanza_t *query;
  query = xmpp_stanza_get_child_by_name(stanza, "pubsub");
  query = xmpp_stanza_get_child_by_name(query, "items");
  query = xmpp_stanza_get_child_by_name(query, "item");
  query = xmpp_stanza_get_child_by_name(query, "bundle");
  query = xmpp_stanza_get_child_by_name(query, "identityKey");
  char* identityKey = xmpp_stanza_get_text(query);

 size_t identity_public_key_len = 0;
  unsigned char *x = g_base64_decode(identityKey, &identity_public_key_len);
  unsigned char *identity_public_key_data = x;
  if (identity_public_key_data[0] != DJB_TYPE) {
    printf("Ist kein DJB_TYPE");
  }

  // Skip first byte corresponding to signal DJB_TYPE
  identity_public_key_len--;
  identity_public_key_data = &identity_public_key_data[1];

  char *fingerprint = malloc(identity_public_key_len * 2 + 1);
  fingerprint[identity_public_key_len*2] = '\0';

  for (int i = 0; i < identity_public_key_len; i++) {
    fingerprint[i * 2] = (identity_public_key_data[i] & 0xf0) >> 4;
    fingerprint[i * 2] += '0';
    if (fingerprint[i * 2] > '9') {
      fingerprint[i * 2] += 0x27;
    }

    fingerprint[(i * 2) + 1] = identity_public_key_data[i] & 0x0f;
    fingerprint[(i * 2) + 1] += '0';
    if (fingerprint[(i * 2) + 1] > '9') {
      fingerprint[(i * 2) + 1] += 0x27;
    }
  }

  printf("xmpp:%s?omemo-sid-%s=%s\n", xmpp_conn_get_jid(conn), xmpp_stanza_get_id(stanza), fingerprint);
  response--;
  if(response == 0) {
    xmpp_disconnect(xmppc->conn);
    xmpp_stop(xmppc->ctx);
  }

  return 0;
}


