/*
 * @file xmppc.h
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

#ifndef XMPPC_XMPPC_H__
#define XMPPC_XMPPC_H__

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <strophe.h>
#include <unistd.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"

#define ANSI_COLOR_B_RED "\x1b[91m"

#define ANSI_COLOR_RESET "\x1b[m"

/**
 * XMPPC Level of Logging
 *
 */
typedef enum loglevel {
  ERROR = 0,
  WARN = 1,
  INFO = 2,
  DEBUG = 3,
  TRACE = 4
} loglevel_t;

typedef enum mode {
  UNKOWN,
  ACCOUNT,
  ROSTER,
  MESSAGE,
  MUC,
  OMEMO,
  PGP,
  OPENPGP,
  MONITOR,
  MAM,
  DISCOVERY,
  BOOKMARK
} xmppc_mode_t;

typedef struct {
  /** log level **/
  loglevel_t loglevel;
  xmpp_ctx_t *ctx;
  xmpp_conn_t *conn;
  xmppc_mode_t mode;
} xmppc_t;

#define INIT_XMPPC(X) xmppc_t X = {.loglevel = ERROR, .ctx = NULL, .conn = NULL, .mode = UNKOWN}

typedef void (*ExecuteHandler)(xmppc_t *, int, char **);

void logError(xmppc_t *xmppc_t, const char *fmt, ...);

void logWarn(xmppc_t *xmppc, const char *fmt, ...);

void logInfo(xmppc_t *xmppc, const char *fmt, ...);

void logDebug(xmppc_t *xmppc, const char *fmt, ...);

int xmppc_context(xmppc_t *xmppc, int level);

int xmppc_connect(xmppc_t *_xmppc, char *jid, char *password);

#endif // XMPPC_XMPPC_H__
