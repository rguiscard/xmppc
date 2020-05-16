/*!
 * @file main.c
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

/*!
 * @mainpage xmppc - XMPP command line
 *
 * @section whatIsXmppc What is xmppc
 *
 * xmppc is a command line program for XMPP (https://xmpp.org/).
 * The program has been written for the Debian GNU/Linux system (https://www.debian.org/).
 *
 * @section build Build
 *
 * @subsection dependencies Dependencies
 *
 * - The GNU C Library (glibc) - 2.28-10
 * - GLib - 2.58.3
 * - XMPP library libstrophe - 0.9.2-2
 * - GPGME (GnuPG Made Easy) - 1.12.0-6
 *
 * @subsection compile Compile
 *
 * @code{.bash}
 * ./bootstrap.sh
 * ./configure
 * make
 * @endcode
 *
 * @subsection documentation Documentation
 * @code{.bash}
 * cd doc
 * make
 * @endcode
 * @section usage Usage
 *
 * @code{.bash}
 * xmppc --jid <jabberid> --pwd <secret> --mode <mode> <command> <command args>
 * @endcode
 *
 * @code{.bash}
 * xmppc --jid user@domain.tld --pwd "my secret" --mode pgp chat friend@domain.tld "Hello!"
 * xmppc --jid user@domain.tld --pwd $(pass XMPP/domain/user) --mode omemo list
 * @endcode
 *
 *
 * @section development Development
 *
 * - \subpage module
 *
 */

/*!
 * \page module Modules
 *
 * \section account Account
 * \section roster Roster
 * \section message Message
 * \section muc Multi-User-Chat
 * \section omemo OMEMO
 * XEP-0384: OMEMO Encryption (Version 0.3.0 (2018-07-31)). xmppc can use used
 * to request the users OMEMO Device ID List and displays the Fingerprints of
 * the keys.
 *
 * @code{.bash}
 * xmppc --jid user@domain.tld --pwd $(pass domain.tld/user) --mode omemo
 * @endcode
 *
 * @code{.bash}
 * xmppc --jid user@domain.tld --pwd $(pass domain.tld/user) --mode omemo| gpg --clear-sign --sign-with 123ABC_MY_OPENPGP_KEY_1234 > omemo.asc
 * @endcode
 *
 *
 * \section pgp PGP
 * XEP-0027: Current Jabber OpenPGP Usage
 * (https://xmpp.org/extensions/xep-0027.html) is obsolete, but there are still
 * clients which supports XEP-0027 instead of XEP-0373 / XEP-0374. This module
 * supports sending of OpenPGP encrypted messages via XMPP's XEP-0027.
 *
 * @code{.bash}
 * xmppc --jid user@domain.tld --pwd "my secret" --mode pgp friend@domain.tld "Hello!"
 * @endcode
 *
 * xmppc use GnuPG's GPGME API use lookup the own key and the key of the
 * recipient. Those two keys will be used to encrypt the message.
 *
 * Details \subpage module-pgp
 *
 */

#include "config.h"
#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <strophe.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "xmppc.h"
#include "mode/account.h"
#include "mode/message.h"
#include "mode/muc.h"
#include "mode/omemo.h"
#include "mode/roster.h"
#include "mode/pgp.h"
#include "mode/openpgp.h"
#include "mode/monitor.h"
#include "mode/mam.h"
#include "mode/discovery.h"
#include "mode/bookmark.h"

/*!
 * @brief The callback structure.
 *
 * This struct is used to call the "execution"-function of the mode module (e.g
 * account_execute_command, roster_execute_command, ...).
 *
 * @authors DebxWoody
 * @since 0.0.2
 * @version 1
 *
 */

typedef struct {
  /*! Size of arguments */
  int argc;
  /*! Array of C-Strings (arguments) with size of argc */
  char **argv;
  /*! Pointer to Execute-Handler */
  ExecuteHandler callback;
  /*! XMPPC context object */
  xmppc_t *xmppc;
} callback_t;

/*!
 * @brief Mode Mapping.
 *
 * This struct is used to provider the mapping of the mode name (name via
 * command line interface of the user (e.g. -m account). The technical ID of the
 * mode which is defined via enum xmppc_mode_t and the pointer of function which
 * should be used to handle those mode.
 *
 * @authors DebxWoody
 * @since 0.0.2
 * @version 1
 */

struct mode_mapping {
  const char *name;
  xmppc_mode_t mode;
  ExecuteHandler callback;
};

/*!
 * @brief Mode mapping table.
 *
 * Mapping of mode's string, enum and function.
 *
 * @authors DebxWoody
 * @since 0.0.2
 * @version 1
 *
 */

static struct mode_mapping map[] = {
  /*!
  * Account Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"account", ACCOUNT, account_execute_command},
  /*!
  * roster Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"roster", ROSTER, roster_execute_command},
  /*!
  * Message Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"message", MESSAGE, message_execute_command},
  /*!
  * MUC Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"muc", MUC, muc_execute_command},
  /*!
  * OMEMO Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"omemo", OMEMO, omemo_execute_command},
  /*!
  * PGP Mode Mapping
  * @since 0.0.2
  * @version 1
  */
  {"pgp", PGP, pgp_execute_command},
  /*!
  * XEP-0373: OpenPGP for XMPP
  */
  {"openpgp", OPENPGP, openpgp_execute_command},
  /*!
  * Monitor
  */
  {"monitor", MONITOR, monitor_execute_command},
  //
  {"discovery", DISCOVERY, discovery_execute_command},
  //
  {"bookmark", BOOKMARK, bookmark_execute_command},
  // 
  {"mam", MAM, mam_execute_command},
  // End of Map
  {NULL, 0}
};

/*!
 * \brief Connection Handler Callback of libstrophe.
 *
 * This function will be called by libstrophe.
 *
 * \param conn See libstrophe documentation
 * \param status See libstrophe documentation
 * \param error See libstrophe documentation
 * \param stream_error See libstrophe documentation
 *
 * \param userdata callback_t object for the mode request by user.
 *
 * @authors DebxWoody
 * @since 0.0.2
 * @version 1
 *
 */

void conn_handler(xmpp_conn_t *const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t *const stream_error,
                  void *const userdata) {
  callback_t *callback = (callback_t *)userdata;

  if( error != 0 ) {
    logError(callback->xmppc,"Connection failed. %s\n", strerror(error));
    xmpp_stop(xmpp_conn_get_context(conn));
    return;
  }

  if( stream_error != NULL ) {
    logError(callback->xmppc,"Connection failed. %s\n", stream_error->text);
    xmpp_stop(xmpp_conn_get_context(conn));
    return;
  }


  switch (status) {
    case XMPP_CONN_CONNECT:
      logInfo(callback->xmppc, "Connected\n");
      if( xmpp_conn_is_secured(conn) ) {
        logInfo(callback->xmppc, "Secure connection!\n");
      } else {
       logWarn(callback->xmppc, "Connection not secure!\n");
      }
      callback->callback(callback->xmppc, callback->argc, callback->argv);
      break;
    case XMPP_CONN_RAW_CONNECT:
    case XMPP_CONN_DISCONNECT:
    case XMPP_CONN_FAIL:
      logInfo(callback->xmppc, "Stopping XMPP!\n");
      xmpp_stop(xmpp_conn_get_context(conn));
  }
}

static void _show_help();

/*!
 * \brief C-Main function
 *
 * - Parse argv arguments of the command line.
 * - Checks jid and password for xmpp logging
 * - Checks mode requested by user
 * - All other arguments will be used as mode parameters
 *
 * \param argc Arguments counter
 * \param argv Arguments vector
 * \returns Exit Code
 *
 * @authors DebxWoody
 * @since 0.0.2
 * @version 2
 */

int main(int argc, char *argv[]) {
#if XMPPC_DEVELOPMENT
  printf("!!! WARNING: XMPPC is running in development mode !!!\n");
#endif

  INIT_XMPPC(xmppc);

  static int verbose_flag = 0;
  int c = 0;
  xmppc_mode_t mode = UNKOWN;
  char *jid = NULL;
  char *pwd = NULL;
  char *account = NULL;

  static struct option long_options[] = {
      /* These options set a flag. */
      {"verbose", no_argument, &verbose_flag, 1},
      {"help", no_argument, 0, 'h'},
      {"config", required_argument, 0, 'c'},
      {"account", required_argument, 0, 'a'},
      {"jid", required_argument, 0, 'j'},
      {"pwd", required_argument, 0, 'p'},
      {"mode", required_argument, 0, 'm'},
      {"file", required_argument, 0, 'f'},
      {0, 0, 0, 0}};
  while (c > -1) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hva:j:p:m:", long_options, &option_index);
    if (c > -1) {
      switch (c) {
      case 'h':
        _show_help();
        return EXIT_SUCCESS;

      case 'c':
        printf("option -c with value `%s'\n", optarg);
        break;

      case 'f':
        printf("option -f with value `%s'\n", optarg);
        break;

      case 'a':
        account = malloc(strlen(optarg) + 1);
        strcpy(account, optarg);
        break;

      case 'j':
        jid = malloc(strlen(optarg) + 1);
        strcpy(jid, optarg);
        break;

      case 'p':
        pwd = malloc(strlen(optarg) + 1);
        strcpy(pwd, optarg);
        break;

      case 'm':
        for(int i = 0; map[i].name;i++ ) {
          if (strcmp(optarg, map[i].name) == 0) {
            mode = map[i].mode;
            break;
          }
        }
        break;

      case 'v':
        verbose_flag++;
        break;

      case '?':
        break;

      default:
        abort();
      }
    }
  }

  // Loading config file
  GKeyFile *config_file = g_key_file_new();
  GError *error = NULL;
  GString* configfile = g_string_new( g_get_home_dir());
  g_string_append(configfile,"/.config/xmppc.conf");
  gboolean configfilefound = g_key_file_load_from_file(
    config_file,
    configfile->str,
    G_KEY_FILE_NONE,
    &error);

  if (!configfilefound) {
   if(!g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
      logError(&xmppc, "Error loading key file: %s", error->message);
      return -1;
    }
  } else {
    if(jid == NULL && pwd == NULL) { 
      logInfo(&xmppc,"Loading default account\n");
      if( account == NULL ) {
        account = "default";
      }
      jid = g_key_file_get_value (config_file, account, "jid" ,&error);
      pwd = g_key_file_get_value (config_file, account, "pwd" ,&error);
    }
  }

  if ( !pwd ) {
    static struct termios current_terminal;
    static struct termios pwd_terminal;

    tcgetattr(STDIN_FILENO, &current_terminal);

    pwd_terminal = current_terminal;
    pwd_terminal.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &pwd_terminal);
    printf("Password for %s:", jid);
    pwd = malloc(sizeof(char) * BUFSIZ);
    if (fgets(pwd, BUFSIZ, stdin) == NULL) {
        pwd[0] = '\0';
    } else {
        pwd[strlen(pwd)-1] = '\0';
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &current_terminal);
  }

  int paramc = argc- optind;
  char* paramv[paramc];

  for (int i = optind; i < argc; i++) {
    char* x= malloc(strlen(argv[i])+1 *sizeof(char)  );
    strcpy(x,argv[i]);
    paramv[i-optind] = x;
  }
  xmppc_context(&xmppc, verbose_flag);

  logInfo(&xmppc, "Connecting %s ... ", jid);
  xmppc_connect(&xmppc, jid, pwd);

  ExecuteHandler handler = NULL;
        for(int i = 0; map[i].name;i++ ) {
          if (mode ==  map[i].mode) {
            handler = map[i].callback;
            break;
          }
        }

  if( handler == NULL ) {
    logError(&xmppc, "Unbekannter mode\n");
    return -1;
  } 

  callback_t callback = {paramc, paramv, handler, &xmppc};

#if XMPPC_DEVELOPMENT
  printf("Warning: Developer-Mode: XMPP_CONN_FLAG_TRUST_TLS\n");
  xmpp_conn_set_flags(xmppc.conn, XMPP_CONN_FLAG_TRUST_TLS);
#else
  xmpp_conn_set_flags(xmppc.conn, XMPP_CONN_FLAG_MANDATORY_TLS);
#endif

  int e =  xmpp_connect_client(xmppc.conn, NULL, 0, conn_handler, &callback);
  if(XMPP_EOK != e ) {
    printf("xmpp_connect_client failed");
  }

  xmpp_run(xmppc.ctx);
  xmpp_conn_release(xmppc.conn);
  xmpp_ctx_free(xmppc.ctx);
  xmpp_shutdown();

  return EXIT_SUCCESS;
}

static void _show_help() {
#ifdef XMPPC_DEVELOPMENT
  printf("%s - Development\n", PACKAGE_STRING);
#else
  printf("%s\n", PACKAGE_STRING);
#endif
  printf("Usage: xmppc [--account <account>] [ --jid <jid> --pwd <pwd>] --mode <mode> <command> [<parameters> ...]\n");
  printf("Options:\n");
  printf("  -h / --help                 Display this information.\n");
  printf("  -j / --jid <jid>            Jabber ID\n");
  printf("  -p / --pwd <password>       Passwort\n");
  printf("  -a / --account <account>    Account\n");
  printf("  -m / --mode <mode>          xmppc mode\n");
  printf("\n");
  printf("Modes:\n");
  printf("  -m --mode roster            xmppc roster mode\n");
  printf("    list                      List all contacts\n");
  printf("    export                    Exports all contacts\n");
  printf("\n");
  printf("  -m --mode message           xmppc message mode\n");
  printf("    chat <jid> <message>      Sending unencrypted message to jid\n");
  printf("\n");
  printf("  -m --mode pgp               xmppc pgp mode (XEP-0027) \n");
  printf("    chat <jid> <message>      Sending pgp encrypted message to jid\n");
  printf("\n");
  printf("  -m --mode omemo             xmppc omemo mode (XEP-0384)\n");
  printf("    list                      List the device IDs and fingerprints\n");
  printf("\n");
  printf("  -m --mode openpgp           xmppc openpgp mode (XEP-0373)\n");
  printf("    signcrypt <jid> <message> Sending pgp signed and encrypted message to jid\n");
  printf("\n");
  printf("  -m --mode monitor           Monitot mode\n");
  printf("    stanza                    Stanza Monitor\n");
  printf("    monitor                   microblog Monitor microblog (XEP-0277)\n");
  printf("\n");
  printf("  -m --mode bookmark          Bookmark mode (XEP-0048)\n");
  printf("    list                      List bookmarks\n");
  printf("\n");
  printf("  -m --mode mam               Message Archive Management (XEP-0313)\n");
  printf("    list <jid>                List messages from <jid>\n");
  printf("\n");
  printf("  -m --mode discovery         Service Discovery (XEP-0030)\n");
  printf("    info <jid>                info request for <jid>\n");
  printf("    item <jid>                item request for <jid>\n");
  printf("\n");
  printf("\n");
  printf("Examples:\n");
  printf("  Usage: xmppc --jid user@domain.tld --pwd \"secret\" --mode roster list\n");
  printf("  Usage: xmppc --jid user@domain.tld --pwd \"secret\" --mode pgp chat friend@domain.tld \"Hello\"\n");
  printf("  Usage: xmppc -a account1 --mode discovery item conference@domain.tld\n");
  printf("  Usage: xmppc --mode bookmark list\n");
}
