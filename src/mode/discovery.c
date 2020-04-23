/*!
 * @file File: discovery.c
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

#include "discovery.h"
#include "stdio.h"
#include "string.h"

static void _discovery_get_info(xmppc_t *xmppc, char* to);

static void _discovery_get_item(xmppc_t *xmppc, char* to);

static int _discovery_info_result_handle(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata);

static int _discovery_item_result_handle(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata);

void discovery_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if( argc == 2 && ( strcmp("info", argv[0] ) == 0)) {
    _discovery_get_info(xmppc, argv[1]);
  } else if( argc == 2 && ( strcmp("item", argv[0] ) == 0)) {
    _discovery_get_item(xmppc, argv[1]);
  } else {
    logError(xmppc, "Befehl unbekannt");
  }
}

static void _discovery_get_info(xmppc_t *xmppc, char* to) {
  char* id = xmpp_uuid_gen(xmppc->ctx);
  xmpp_stanza_t *iq = xmpp_iq_new(xmppc->ctx, "get", id);
  xmpp_stanza_set_to(iq,to);
  xmpp_stanza_set_from(iq, xmpp_conn_get_jid(xmppc->conn));
  xmpp_stanza_t *query = xmpp_stanza_new(xmppc->ctx);
  xmpp_stanza_set_name(query,"query");
  xmpp_stanza_set_ns(query,"http://jabber.org/protocol/disco#info");
  xmpp_stanza_add_child(iq, query);
  xmpp_id_handler_add(xmppc->conn, _discovery_info_result_handle, id, xmppc);
  xmpp_send(xmppc->conn, iq);  
}

static void _discovery_get_item(xmppc_t *xmppc, char* to) {
  char* id = xmpp_uuid_gen(xmppc->ctx);
  xmpp_stanza_t *iq = xmpp_iq_new(xmppc->ctx, "get", id);
  xmpp_stanza_set_to(iq,to);
  xmpp_stanza_set_from(iq, xmpp_conn_get_jid(xmppc->conn));
  xmpp_stanza_t *query = xmpp_stanza_new(xmppc->ctx);
  xmpp_stanza_set_name(query,"query");
  xmpp_stanza_set_ns(query,"http://jabber.org/protocol/disco#items");
  xmpp_stanza_add_child(iq, query);
  xmpp_id_handler_add(xmppc->conn, _discovery_item_result_handle, id, xmppc);
  xmpp_send(xmppc->conn, iq);  
}

static int _discovery_item_result_handle(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata) {
  xmppc_t *xmppc = (xmppc_t *)userdata;
 
  logInfo(xmppc,"Discover Item for: %s\n", xmpp_stanza_get_from(stanza));

  if (strcmp(xmpp_stanza_get_type(stanza), "error") == 0) {
    char* text;
    size_t textlen;
    xmpp_stanza_to_text(stanza,&text, &textlen);
    logError(xmppc, "%s%s%s\n", ANSI_COLOR_RED, text, ANSI_COLOR_RESET);
  }

  xmpp_stanza_t* query = xmpp_stanza_get_child_by_name(stanza, "query");
  if(query != NULL ) {
    xmpp_stanza_t* c = xmpp_stanza_get_children(query);
    while (c) {
      if( strcmp( xmpp_stanza_get_name(c), "item" ) == 0) {
        printf("%-40s - %-40s\n", xmpp_stanza_get_attribute(c, "jid"),  xmpp_stanza_get_attribute(c, "name"));
      } 
      c = xmpp_stanza_get_next(c);
    }
  }
  xmpp_disconnect(xmppc->conn);
  return 0;
}

static int _discovery_info_result_handle(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata){
  xmppc_t *xmppc = (xmppc_t *)userdata;
  
  if (strcmp(xmpp_stanza_get_type(stanza), "error") == 0) {
    char* text;
    size_t textlen;
    xmpp_stanza_to_text(stanza,&text, &textlen);
    logError(xmppc, "%s%s%s\n", ANSI_COLOR_RED, text, ANSI_COLOR_RESET);
    return 0;
  }

  logInfo(xmppc,"Discover Info for:\t%s\n", xmpp_stanza_get_from(stanza) );

  xmpp_stanza_t* query = xmpp_stanza_get_child_by_name(stanza, "query");
  if(query != NULL ) {
    xmpp_stanza_t* c = xmpp_stanza_get_children(query);
    
    while (c) {
      if( strcmp( xmpp_stanza_get_name(c), "identity" ) == 0) {
        printf("%-10s - %-20s - %-40s\n", xmpp_stanza_get_attribute(c, "type"),xmpp_stanza_get_attribute(c, "category"), xmpp_stanza_get_attribute(c, "name")  );
      } else if( strcmp( xmpp_stanza_get_name(c), "feature" ) == 0) {
        if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_passwordprotected") == 0  ) printf("\tpassword protected\n");
        else if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_hidden") == 0  ) printf("\tmuc_hidden\n");
        else if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_temporary") == 0  ) printf("\ttemporary\n");
        else if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_open") == 0  ) printf("\topen\n");
        else if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_unmoderated") == 0  ) printf("\tunmoderated\n");
        else if ( strcmp( xmpp_stanza_get_attribute(c, "var"), "muc_nonanonymous") == 0  ) printf("\tnonanonymous\n");
        else printf("\t%-10s\n", xmpp_stanza_get_attribute(c, "var"));
      }
      c = xmpp_stanza_get_next(c);
    }
    
  }

  xmpp_disconnect(xmppc->conn);

  return 0;
}

