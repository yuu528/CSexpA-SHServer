#include "parser.h"
#include "fileutil.h"
#include "session.h"

#include <stdio.h>
#include <string.h>

#define HEADER_MAX_LEN_PER_LINE 128
#define HEADER_MAX_ARGS 50
#define HTACCESS_MAX_ARGS 50

int parse_header(char *buf, int size, session_info *info) {
  char line[HEADER_MAX_LEN_PER_LINE], *ptok, *ptokn;
  char *delim = "\n";
  int i = 0;

  // init
  info->auth_type = E_AUTH_TYPE_NONE;
  strcpy(info->client_authorization, "");

  ptok = strtok_r(buf, delim, &ptokn);

  while (ptok != NULL) {
    // remove \n
    for (int j = 0; j < strlen(ptok); j++) {
      if (ptok[j] == '\n') {
        ptok[j] = '\0';
      }
    }

    if (i == 0) {
      parse_status(ptok, info);
    } else {
      parse_header_field(ptok, info);
    }

    ptok = strtok_r(NULL, delim, &ptokn);

    i++;
  }

  check_file(info);
  find_htaccess(info);
  parse_htaccess(info);

  return -1;
}

void parse_status(char *status, session_info *pinfo) {
  char cmd[1024];
  char path[1024];
  char *pext;
  int i, j;

  enum state_type { SEARCH_CMD, SEARCH_PATH, SEARCH_END } state;

  state = SEARCH_CMD;
  j = 0;
  for (i = 0; i < strlen(status); i++) {
    switch (state) {
    case SEARCH_CMD:
      if (status[i] == ' ') {
        cmd[j] = '\0';
        j = 0;
        state = SEARCH_PATH;
      } else {
        cmd[j] = status[i];
        j++;
      }
      break;

    case SEARCH_PATH:
      if (status[i] == ' ') {
        path[j] = '\0';
        j = 0;
        state = SEARCH_END;
      } else {
        path[j] = status[i];
        j++;
      }
      break;
    }
  }

  strcpy(pinfo->cmd, cmd);
  strcpy(pinfo->path, path);
}

void parse_header_field(char *line, session_info *info) {
  char args[HEADER_MAX_ARGS][1024];
  char *ptok, *ptokn, *ptok2, *ptokn2;
  char *delim = ":", *delim2 = " ";
  int i = 0, j;

  ptok = strtok_r(line, delim, &ptokn);

  while (ptok != NULL) {
    strcpy(args[i], ptok);

    ptok = strtok_r(NULL, delim, &ptokn);

    i++;
  }

  // parse args
  if (strcmp(args[0], "Authorization") == 0) {
    j = 0;

    // skip space
    while (args[1][j] == ' ') {
      j++;
    }

    ptok2 = strtok_r(args[1] + j, delim2, &ptokn2);

    // check auth type
    if (strcmp(args[1] + j, "Basic") == 0) {
      // remove \r
      for (int k = 0; k < strlen(ptok2); k++) {
        if (ptok2[k] == '\r' || ptok2[k] == '\n') {
          ptok2[k] = '\0';
        }
      }

      strcpy(info->client_authorization, ptokn2);
    }
  }
}

void parse_htaccess(session_info *info) {
  FILE *fp;
  char buf[1024];
  char args[HTACCESS_MAX_ARGS][1024];

  char *delim = " \t";
  char *ptok, *ptokn;

  int i, j = 0;
  int argc;
  int ret;

  // apply from htaccess in parent dir
  for (i = info->htaccess_count - 1; i >= 0; i--) {
    fp = fopen(info->htaccess_paths[i], "r");

    if (fp == NULL) {
      continue;
    }

    while (fgets(buf, 1024, fp) != NULL) {
      if (buf[0] == '#') {
        continue;
      }

      // remove \n, \r
      for (int k = 0; k < strlen(buf); k++) {
        if (buf[k] == '\n' || buf[k] == '\r') {
          buf[k] = '\0';
        }
      }

      j = 0;

      ptok = strtok_r(buf, delim, &ptokn);

      while (ptok != NULL && j < HTACCESS_MAX_ARGS) {
        strcpy(args[j], ptok);

        j++;
        ptok = strtok_r(NULL, delim, &ptokn);
      }

      argc = j;

      // parse args
      if (strcmp(args[0], "Redirect") == 0) {
        if (argc < 4) {
          continue;
        }

        if (strcmp(args[1], "parmanent") == 0) {
          info->code = 301;
        } else if (strcmp(args[1], "temp") == 0) {
          info->code = 302;
        } else if (strcmp(args[1], "seeother") == 0) {
          info->code = 303;
        }

        strcpy(info->location, args[3]);
      } else if (strcmp(args[0], "AuthUserFile") == 0) {
        if (argc < 2) {
          continue;
        }

        // convert to relative path from htaccess
        if (args[1][0] != '/') {
          char *path = info->htaccess_paths[i];
          char parent[MAX_PATH_LEN];
          get_parent_path(path, parent);

          strcpy(info->auth_user_file, parent);
          strcat(info->auth_user_file, args[1]);
        } else {
          strcpy(info->auth_user_file, args[1]);
        }
      } else if (strcmp(args[0], "AuthName") == 0) {
        if (argc < 2) {
          continue;
        }

        // copy after remaining string
        strcpy(info->auth_name, "");
        for (int k = 1; k < argc; k++) {
          // remove "
          for (int l = 0; l < strlen(args[k]); l++) {
            if (args[k][l] == '"') {
              args[k][l] = ' ';
            }
          }

          strcat(info->auth_name, args[k]);

          if (k != argc - 1) {
            strcat(info->auth_name, " ");
          }
        }
      } else if (strcmp(args[0], "AuthType") == 0) {
        if (argc < 2) {
          continue;
        }

        if (strcmp(args[1], "Basic") == 0) {
          info->auth_type = E_AUTH_TYPE_BASIC;
        }
      }
    }

    fclose(fp);
  }
}
