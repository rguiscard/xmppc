/*
 * @file xmppc.c
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

#include "xmppc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

/*! @file xmppc.c
 *
 * xmppc
 *
 */

/*!
 * Error logging
 *
 * \param xmppc the xmppc context structure
 * \param fmt format of message
 *
 */
void logError(xmppc_t *xmppc, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void logWarn(xmppc_t *xmppc, const char *fmt, ...) {
  if (xmppc->loglevel) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}

void logInfo(xmppc_t *xmppc, const char *fmt, ...) {
  if (xmppc->loglevel) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}

void logDebug(xmppc_t *xmppc, const char *fmt, ...) {
  if (xmppc->loglevel) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}

/*!
 * \brief Setup the application context.
 *
 * \param xmppc
 * \return 0 - ok
 *
 **/
int xmppc_context(xmppc_t *xmppc, int level) {
  assert(xmppc != NULL);
  assert(xmppc->ctx == NULL);
  xmpp_log_t *log = NULL;

  if (level > TRACE) {
    logError(xmppc, "Log level %d not supported. Max: %d.\n", level, TRACE);
    exit(-1);
  }

  xmppc->loglevel = level;

  if (level == ERROR) {
    log = xmpp_get_default_logger(XMPP_LEVEL_ERROR);
  } else if (level == WARN) {
    log = xmpp_get_default_logger(XMPP_LEVEL_WARN);
  } else if (level == INFO) {
    log = xmpp_get_default_logger(XMPP_LEVEL_INFO);
  } else if (level == DEBUG) {
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
  } else if (level == TRACE) {
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
  }

  xmpp_ctx_t *ctx = xmpp_ctx_new(NULL, log);
  xmppc->ctx = ctx;
  return 0;
}

int xmppc_connect(xmppc_t *xmppc, char *jid, char *password) {
  assert(xmppc != NULL);
  assert(jid != NULL);
  assert(password != NULL);
  xmpp_conn_t *conn = xmpp_conn_new(xmppc->ctx);
  assert(conn != NULL);

  xmpp_conn_set_jid(conn, jid);
  xmpp_conn_set_pass(conn, password);
  xmppc->conn = conn;
  return 0;
}
