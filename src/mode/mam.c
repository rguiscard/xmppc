/*!
 * @file File: mam.c
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

#include "mam.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

static void _mam_request(xmppc_t *xmppc, char* to, bool pretty);
static int _mam_show(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);
static int _mam_display_message(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);
static int _mam_display_pretty_message(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata);

void mam_execute_command (xmppc_t *xmppc, int argc, char *argv[]) {
  if(argc == 2) {
    if (strcmp("list", argv[0]) == 0) {
      _mam_request(xmppc, argv[1], false);
      return;
    } else if (strcmp("pretty", argv[0]) == 0) {
      _mam_request(xmppc, argv[1], true);
      return;
    }
  }

  printf("Command unbekannt!");
  xmpp_disconnect(xmppc->conn);
}

static void _mam_request(xmppc_t *xmppc, char* to, bool pretty) {

  if (pretty) {
    xmpp_handler_add (xmppc->conn, _mam_display_pretty_message, NULL,"message",NULL, xmppc);
  } else {
    xmpp_handler_add (xmppc->conn, _mam_display_message, NULL,"message",NULL, xmppc);
  }

    char* id = xmpp_uuid_gen(xmppc->ctx);
    xmpp_stanza_t *iq = xmpp_iq_new(xmppc->ctx, "set", id);
    // query
    xmpp_stanza_t* query = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(query, "query");
    xmpp_stanza_set_ns(query, "urn:xmpp:mam:2");

  
    xmpp_stanza_t* x = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(x,"x");
    xmpp_stanza_set_ns(x, "jabber:x:data"),
    xmpp_stanza_set_type(x,"submit");
    
    xmpp_stanza_add_child(query,x);

    xmpp_stanza_t* f = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(f,"field");
    xmpp_stanza_set_type(f,"hidden");
    xmpp_stanza_set_attribute(f, "var", "FORM_TYPE");
    xmpp_stanza_t* v = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(v,"value");
    xmpp_stanza_t* b = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_text(b, "urn:xmpp:mam:2");
    
    xmpp_stanza_add_child(x,f);
    xmpp_stanza_add_child(f,v);
    xmpp_stanza_add_child(v,b);

    f = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(f,"field");
    xmpp_stanza_set_attribute(f, "var", "with");
    v = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_name(v,"value");
    b = xmpp_stanza_new(xmppc->ctx);
    xmpp_stanza_set_text(b, to);
    //xmpp_stanza_set_text(b, xmpp_conn_get_jid (xmppc->conn));

    xmpp_stanza_add_child(x,f);
    xmpp_stanza_add_child(f,v);
    xmpp_stanza_add_child(v,b);

    xmpp_stanza_add_child(iq,query);
    xmpp_id_handler_add(xmppc->conn, _mam_show, id, xmppc);
    xmpp_send(xmppc->conn,iq);

    free(id);
}

int _mam_show(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata){
  xmpp_stanza_t *fin = xmpp_stanza_get_child_by_name(stanza,"fin");
  if(fin) {
    xmpp_disconnect(conn);
  }
  return 1;
}


static int _mam_display_message(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata) {
  xmpp_stanza_t* result = xmpp_stanza_get_child_by_name(stanza,"result");
  if( result && strcmp("urn:xmpp:mam:2", xmpp_stanza_get_ns(result)) == 0 ) {
    char* s;
    size_t len;
    xmpp_stanza_to_text(xmpp_stanza_get_children(result) ,&s,&len);
    printf("%s%s\x1b[m\n", ANSI_COLOR_YELLOW, s);
  }
  return 1;
}

static int _mam_display_pretty_message(xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata)
{
  xmpp_stanza_t* result = xmpp_stanza_get_child_by_name(stanza,"result");

  if( result && strcmp("urn:xmpp:mam:2", xmpp_stanza_get_ns(result)) == 0 ) {

    xmpp_stanza_t* forwarded = xmpp_stanza_get_child_by_ns(result, "urn:xmpp:forward:0");
    if (forwarded) {

      xmpp_stanza_t* message = xmpp_stanza_get_child_by_name(forwarded, "message");
      if (message) {
        const char* const from = xmpp_stanza_get_from(message);
        xmpp_stanza_t* body = xmpp_stanza_get_child_by_name(message, "body");

        if (body) {
          char *text = xmpp_stanza_get_text(body);
          printf("%s%s\x1b[m: %s\n", ANSI_COLOR_YELLOW, from, text);
          free(text);
        }
      }
    }
  }

  return 1;
}
