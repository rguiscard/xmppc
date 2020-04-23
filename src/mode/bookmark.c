/*!
 * @file File: bookmark.c
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

#include "bookmark.h"
#include "stdio.h"
#include "string.h"

static void _bookmark_request(xmppc_t *xmppc);
static int _bookmark_show(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);

void bookmark_execute_command (xmppc_t *xmppc, int argc, char *argv[]) {
  if(argc == 1 && strcmp("list", argv[0]) == 0) {
    _bookmark_request(xmppc);
  } else {
    printf("Command unbekannt!");
    xmpp_disconnect(xmppc->conn);
  }
}

static void _bookmark_request(xmppc_t *xmppc) {
    char* id = xmpp_uuid_gen(xmppc->ctx);
    xmpp_stanza_t *iq = xmpp_iq_new(xmppc->ctx, "get", id);
    // pubsub
    xmpp_stanza_t* pubsub = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(pubsub, "pubsub");
    xmpp_stanza_set_ns(pubsub, "http://jabber.org/protocol/pubsub");
    // items
    xmpp_stanza_t* items = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(items, "items");
    xmpp_stanza_set_attribute(items, "node", "storage:bookmarks");

    xmpp_stanza_add_child(iq,pubsub);
    xmpp_stanza_add_child(pubsub,items);
    xmpp_id_handler_add(xmppc->conn, _bookmark_show, id, xmppc);
    xmpp_send(xmppc->conn,iq);

}

int _bookmark_show(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata){
  xmpp_stanza_t* pubsub = xmpp_stanza_get_child_by_name(stanza,"pubsub");
  xmpp_stanza_t* items = xmpp_stanza_get_child_by_name(pubsub,"items");
  xmpp_stanza_t* item = xmpp_stanza_get_child_by_name(items,"item");
  xmpp_stanza_t* storage = xmpp_stanza_get_child_by_name(item,"storage");
  if(storage != NULL) { 
    xmpp_stanza_t* c = xmpp_stanza_get_children(storage);
    while (c) {
      printf("%s%-50s - %-5s - %-40s\x1b[m\n", ANSI_COLOR_RED, 
        xmpp_stanza_get_attribute(c,"jid"),
        xmpp_stanza_get_attribute(c,"autojoin"),
        xmpp_stanza_get_attribute(c,"name")
        );
      c = xmpp_stanza_get_next(c);
    }
  }

  xmpp_disconnect(conn);
  return 1;
}


