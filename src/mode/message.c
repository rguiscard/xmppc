/*!
 * @file message.c
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

#include <stdlib.h>
#include <string.h>

#include "message.h"

static void _message_send_chat(xmppc_t *xmppc, char* to, char* message);
static void _message_send_groupchat(xmppc_t *xmppc, char* room, char* text, void (*on_join)(xmppc_t *), void (*on_message_sent)(xmppc_t *));

// Callback functions
void on_join_callback(xmppc_t *xmppc) {
    // Actions after joining a group chat
    logDebug(xmppc, "Joined group chat\n");
}

void on_message_sent_callback(xmppc_t *xmppc) {
    // Actions after sending a message
    logDebug(xmppc, "Message sent to group chat\n");
}

void message_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if (argc == 3) {
    if (strcmp("groupchat", argv[0]) == 0) {
      _message_send_groupchat(xmppc, argv[1], argv[2], on_join_callback, on_message_sent_callback);
    } else if (strcmp("chat", argv[0]) == 0) {
      _message_send_chat(xmppc, argv[1], argv[2]);
    } else {
      logError(xmppc, "Unknown command\n");
      return;
    }
  } else {
    logError(xmppc, "Unknown command\n");
  }
  sleep(10);
  xmpp_disconnect(xmppc->conn);
}

void _message_send_chat(xmppc_t *xmppc, char* to, char* text) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *message;
  char* id = xmpp_uuid_gen(xmppc->ctx);
  message = xmpp_message_new(xmpp_conn_get_context(conn), "chat", to, id);
  int res = xmpp_message_set_body(message, text);
  if(res == 0) {
    xmpp_send(conn, message); 
  }
  free(id); 
}

void _message_send_groupchat(xmppc_t *xmppc, char* room, char* text, void (*on_join)(xmppc_t *), void (*on_message_sent)(xmppc_t *)) {
    xmpp_conn_t *conn = xmppc->conn;
    xmpp_ctx_t *ctx = xmpp_conn_get_context(conn);
    
    //JID
    const char* jid = xmpp_conn_get_jid(conn);

    // Joining the room with <x> element
    xmpp_stanza_t *pres = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(pres, "presence");
    char *full_room_jid = malloc(strlen(room) + strlen(jid) + 2); // +2 for '/' and '\0'
    if (!full_room_jid) {
        logError(xmppc, "Memory allocation error\n");
        return;
    }
    sprintf(full_room_jid, "%s/%s", room, jid);
    xmpp_stanza_set_attribute(pres, "to", full_room_jid);

    // Creating <x> element for MUC protocol
    xmpp_stanza_t *x = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(x, "x");
    xmpp_stanza_set_ns(x, "http://jabber.org/protocol/muc");
    xmpp_stanza_add_child(pres, x);
    xmpp_stanza_release(x);

    xmpp_send(conn, pres);
    xmpp_stanza_release(pres);
    free(full_room_jid);

    // Call callback function after joining the room
    if (on_join != NULL) {
        on_join(xmppc);
    }

    // Sending the message
    xmpp_stanza_t *message = xmpp_message_new(ctx, "groupchat", room, NULL);
    if (!message) {
        logError(xmppc, "Failed to create message stanza\n");
        return;
    }

    int res = xmpp_message_set_body(message, text);
    if (res == 0) {
        xmpp_send(conn, message);
        // Call callback function after message is sent
        if (on_message_sent != NULL) {
            on_message_sent(xmppc);
        }
    } else {
        logError(xmppc, "Error setting message body\n");
    }

    xmpp_stanza_release(message);
}
