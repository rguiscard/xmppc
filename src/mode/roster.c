/*
 * @file roster.c
 *
 * vim: expandtab:ts=2:sts=2:sw=2
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

#include "roster.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define COMMAND_LIST "list"
#define COMMAND_EXPORT "export"

typedef enum commandTyp { C_UNKOWN, LIST, EXPORT } CommandType;

typedef struct command {
  CommandType type;
} command_t;

static void _roster_parse_command(command_t *command, int argc, char *argv[]);

static void _roster_send_query(xmppc_t *xmppc,command_t *command);

static int _handle_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata);

static void _teardown(xmppc_t *xmppc);

void roster_execute_command(xmppc_t *xmppc, int argc, char *argv[]) {
  command_t *command = malloc(sizeof(command_t)); 
  command->type = UNKOWN;

  if (argc == 0) {
    logError(xmppc, "No subcommand provided\n");
  } else {
    _roster_parse_command(command, argc, argv);
    _roster_send_query(xmppc, command);
  }

  _teardown(xmppc);
}

static void _roster_parse_command(command_t *command, int argc, char *argv[]) {
  if (strcmp(COMMAND_LIST, argv[0]) == 0) {
    command->type = LIST;
  } else if (strcmp(COMMAND_EXPORT, argv[0]) == 0) {
    command->type = EXPORT;
  }
}

static void _roster_send_query(xmppc_t *xmppc, command_t *command ) {
  logInfo(xmppc, "Send roster query\n");
  xmpp_ctx_t *ctx = xmppc->ctx;
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_stanza_t *iq, *query;
  iq = xmpp_iq_new(ctx, "get", "roster1");
  query = xmpp_stanza_new(ctx);
  xmpp_stanza_set_name(query, "query");
  xmpp_stanza_set_ns(query, XMPP_NS_ROSTER);
  xmpp_stanza_add_child(iq, query);
  xmpp_stanza_release(query);
  xmpp_id_handler_add(conn, _handle_reply, "roster1",command);
  xmpp_send(conn, iq);
  xmpp_stanza_release(iq);
}

int _handle_reply(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza,
                  void *const userdata) {
  command_t* command = (command_t*) userdata;
  xmpp_stanza_t *query, *item;
  const char *type, *name;
  type = xmpp_stanza_get_type(stanza);
  if (strcmp(type, "error") == 0)
    printf("query failed\n");
  else {
    query = xmpp_stanza_get_child_by_name(stanza, "query");
    for (item = xmpp_stanza_get_children(query); item;
         item = xmpp_stanza_get_next(item)) {
      if(command->type == LIST) {
      if ((name = xmpp_stanza_get_attribute(item, "name"))) 
        printf("\t %s (%s) sub=%s\n", name,
               xmpp_stanza_get_attribute(item, "jid"),
               xmpp_stanza_get_attribute(item, "subscription"));
      else
        printf("\t %s sub=%s\n", xmpp_stanza_get_attribute(item, "jid"),
               xmpp_stanza_get_attribute(item, "subscription"));
      } else if (command->type == EXPORT) {
        printf("%s\n", xmpp_stanza_get_attribute(item, "jid"));
      }
    }
  }
  return 0;
}

static void _teardown(xmppc_t *xmppc) {
  xmpp_conn_t *conn = xmppc->conn;
  xmpp_disconnect(conn);
  xmpp_stop(xmpp_conn_get_context(conn));
}
