#ifndef _H_SESSION_

#define _H_SESSION_

#define MAX_PATH_LEN 512
#define MAX_CMD_LEN 64
#define MAX_TYPE_LEN 64

#define HTACCESS_MAX 10

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
} session_info;

int http_session(int);
void http_reply(int sock, session_info *);

#endif
