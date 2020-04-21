/*!
 * @file File: monitor.c
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

#include "monitor.h"
#include "stdio.h"
#include "string.h"

static int _monitor_log_stanza(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);
static int _monitor_show_microblog(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);

void monitor_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if(argc > 0 && strcmp("stanza", argv[0]) == 0) {
    xmpp_handler_add(xmppc->conn, _monitor_log_stanza, NULL, NULL, NULL, NULL);
#ifdef XMPPC_DEVELOPMENT 
  } else if (argc > 0 && strcmp("microblog", argv[0]) == 0) {
    xmpp_handler_add(xmppc->conn, _monitor_show_microblog, NULL, NULL, NULL, NULL);
#endif
  } else {
    printf("Command unbekannt!");
    xmpp_disconnect(xmppc->conn);
  }
    // presence 
    xmpp_stanza_t *presence = xmpp_presence_new(xmppc->ctx);
    char* id = xmpp_uuid_gen(xmppc->ctx);
    xmpp_stanza_set_id(presence, id);
    xmpp_send(xmppc->conn, presence);
    xmpp_stanza_release(presence);

    // XEP-0280: Message Carbons
    xmpp_stanza_t *carbons = xmpp_iq_new(xmppc->ctx, "set", xmpp_uuid_gen(xmppc->ctx));
    xmpp_stanza_t* enable = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(enable, "enable");
    xmpp_stanza_set_ns(enable, "urn:xmpp:carbons:2");
    xmpp_stanza_add_child(carbons,enable);
    xmpp_send(xmppc->conn,carbons);
}

int _monitor_log_stanza(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata){
  char* s;
  size_t len;
  xmpp_stanza_to_text(stanza,&s,&len);

  char* color = "\x1b[m";
  if(strcmp(xmpp_stanza_get_name(stanza), "iq") == 0 ) color = ANSI_COLOR_YELLOW;
  if(strcmp(xmpp_stanza_get_name(stanza), "message") == 0 ) {
    color = ANSI_COLOR_BLUE;
     if(strcmp("headline", xmpp_stanza_get_type(stanza)) == 0) {
      color = ANSI_COLOR_CYAN;
    }
  }

  if(strcmp(xmpp_stanza_get_name(stanza), "presence") == 0 ) color = ANSI_COLOR_GREEN;
  if(xmpp_stanza_get_type(stanza) != NULL) 
  if(strcmp(xmpp_stanza_get_type(stanza), "error") == 0 ) color = ANSI_COLOR_B_RED;

  printf("%s%s\x1b[m\n", color, s);
  return 1;
}

/* XEP-0277: Microblogging over XMPP */
int _monitor_show_microblog(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata){
  xmpp_stanza_t *log = stanza;
  if(strcmp(xmpp_stanza_get_name(stanza), "message") == 0 ) {
    xmpp_stanza_t* event = xmpp_stanza_get_child_by_name(stanza, "event");
    if(event != NULL && 
      strcmp(xmpp_stanza_get_ns(event), "http://jabber.org/protocol/pubsub#event") ==0) {
      char* body_text = xmpp_message_get_body(stanza);
      printf("Event: %s\n", body_text);
      xmpp_stanza_t * items = xmpp_stanza_get_child_by_name(event,"items");
      if( items != NULL && strcmp("urn:xmpp:microblog:0", xmpp_stanza_get_attribute(items, "node" )) == 0) {
      printf("urn:xmpp:microblog:0 items found!\n");
      }
    }
  }

  if(log) {
    _monitor_log_stanza(conn, stanza, userdata);
  }

  return 1;
}

