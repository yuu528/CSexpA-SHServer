#ifndef _H_HTACCESS_PARSER_

#define _H_HTACCESS_PARSER_

#include "session.h"

#define HTACCESS_MAX_KEY_LEN 64
#define HTACCESS_MAX_ARG_LEN 512
#define HTACCESS_MAX_ARGS 50

#define HTACCESS_REDIRECT "Redirect"
#define HTACCESS_REDIRECT_PERM "permanent"
#define HTACCESS_REDIRECT_TEMP "temp"
#define HTACCESS_REDIRECT_SEE_OTHER "seeother"

#define HTACCESS_AUTH_USER_FILE "AuthUserFile"
#define HTACCESS_AUTH_NAME "AuthName"
#define HTACCESS_AUTH_TYPE "AuthType"

#define HTACCESS_ERRORDOC "ErrorDocument"

void parse_htaccess(session_info *);
int parse_raw_args(char *, int, int max_argv_len, char args[][max_argv_len]);
int parse_htaccess_redirect(session_info *, int, int max_argv_len,
                            char args[][max_argv_len]);
int parse_htaccess_auth_user_file(session_info *, int, int max_argv_len,
                                  char[][max_argv_len], int);
int parse_htaccess_auth_name(session_info *, char *);
int parse_htaccess_auth_type(session_info *, char *);
int parse_htaccess_errordoc(session_info *, int, int max_argv_len,
                            char args[][max_argv_len], int);

#endif
