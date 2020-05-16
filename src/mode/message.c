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

#include "message.h"
#include "string.h"

static void _message_send_text(xmppc_t *xmppc, char* to, char* message);

void message_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  if( argc == 3 && ( strcmp("chat", argv[0] ) == 0)) {
    _message_send_text(xmppc, argv[1],argv[2]);
  } else {
    logError(xmppc, "Befehl unbekannt");
  }
  sleep(10);
  xmpp_disconnect(xmppc->conn);
}

void _message_send_text(xmppc_t *xmppc, char* to, char* text) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *message;
  char* id = xmpp_uuid_gen(xmppc->ctx);
  message = xmpp_message_new(xmpp_conn_get_context(conn), "chat", to, id);
  int res = xmpp_message_set_body(message, text);
  if(res == 0) {
    xmpp_send(conn, message); 
  }
}


