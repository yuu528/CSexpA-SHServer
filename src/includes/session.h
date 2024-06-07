#ifndef _H_SESSION_

#define _H_SESSION_

#define MAX_PATH_LEN 512
#define MAX_CMD_LEN 64
#define MAX_QUERY_LEN 1024
#define MAX_TYPE_LEN 64
#define MAX_AUTH_NAME_LEN 512
#define MAX_AUTH_CRED_LEN 2048
#define MAX_HOST_LEN 64

#define HTACCESS_MAX 10

typedef enum { E_AUTH_TYPE_NONE, E_AUTH_TYPE_BASIC } auth_type_spec;

typedef struct {
  char cmd[MAX_CMD_LEN];
  char path[MAX_PATH_LEN];
  char query[MAX_QUERY_LEN];

  char real_path[MAX_PATH_LEN];
  char type[MAX_TYPE_LEN];

  int code;
  int size;

  char content[MAX_QUERY_LEN];
  char client_type[MAX_TYPE_LEN];
  int client_size;

  // cgi
  int is_cgi;

  // htaccess
  char htaccess_paths[HTACCESS_MAX][MAX_PATH_LEN];
  int htaccess_count;

  // redirect
  char location[MAX_PATH_LEN];

  // basic auth
  char auth_user_file[MAX_PATH_LEN];
  char auth_name[MAX_AUTH_NAME_LEN];
  auth_type_spec auth_type;
  char client_authorization[MAX_AUTH_CRED_LEN];

  // errordoc
  char doc_401[MAX_PATH_LEN];
  char doc_403[MAX_PATH_LEN];
  char doc_404[MAX_PATH_LEN];
} session_info;

int http_session(int);
void http_reply(int sock, session_info *);

#endif
