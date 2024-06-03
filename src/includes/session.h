#ifndef _H_SESSION_

#define _H_SESSION_

#define MAX_PATH_LEN 512
#define MAX_CMD_LEN 64
#define MAX_TYPE_LEN 64
#define MAX_AUTH_NAME_LEN 512

#define HTACCESS_MAX 10

typedef enum { E_AUTH_TYPE_NONE, E_AUTH_TYPE_BASIC } auth_type_spec;

typedef struct {
  char cmd[MAX_CMD_LEN];
  char path[MAX_PATH_LEN];

  char real_path[MAX_PATH_LEN];
  char type[MAX_TYPE_LEN];

  int code;
  int size;

  // htaccess
  char htaccess_paths[HTACCESS_MAX][MAX_PATH_LEN];
  int htaccess_count;

  // redirect
  char location[MAX_PATH_LEN];

  // basic auth
  char auth_user_file[MAX_PATH_LEN];
  char auth_name[MAX_AUTH_NAME_LEN];
  auth_type_spec auth_type;
} session_info;

int http_session(int);
void http_reply(int sock, session_info *);

#endif
