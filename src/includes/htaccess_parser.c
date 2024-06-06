#include "htaccess_parser.h"
#include "fileutil.h"
#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_htaccess(session_info *info) {
  FILE *fp;
  char buf[FILE_BUFFER_SIZE];
  char args[HTACCESS_MAX_ARGS][HTACCESS_MAX_ARG_LEN];

  char *delim = " \t";
  char *ptok, *ptokn;

  int i;
  int argc;

  // apply from htaccess in parent dir
  for (i = info->htaccess_count - 1; i >= 0; i--) {
    fp = fopen(info->htaccess_paths[i], "r");

    if (fp == NULL) {
      continue;
    }

    while (fgets(buf, FILE_BUFFER_SIZE, fp) != NULL) {
      if (buf[0] == '#') {
        continue;
      }

      // remove \n, \r
      for (int k = 0; k < strlen(buf); k++) {
        if (buf[k] == '\n' || buf[k] == '\r') {
          buf[k] = '\0';
        }
      }

      ptok = strtok_r(buf, delim, &ptokn);

      if (ptok == NULL) {
        continue;
      }

      // parse args
      argc =
          parse_raw_args(ptokn, HTACCESS_MAX_ARGS, HTACCESS_MAX_ARG_LEN, args);

      if (strcmp(ptok, HTACCESS_REDIRECT) == 0) {
        parse_htaccess_redirect(info, argc, HTACCESS_MAX_ARG_LEN, args);
      } else if (strcmp(ptok, HTACCESS_AUTH_USER_FILE) == 0) {
        parse_htaccess_auth_user_file(info, argc, HTACCESS_MAX_ARG_LEN, args,
                                      i);
      } else if (strcmp(ptok, HTACCESS_AUTH_NAME) == 0) {
        parse_htaccess_auth_name(info, ptokn);
      } else if (strcmp(ptok, HTACCESS_AUTH_TYPE) == 0) {
        parse_htaccess_auth_type(info, ptokn);
      } else if (strcmp(ptok, HTACCESS_ERRORDOC) == 0) {
        parse_htaccess_errordoc(info, argc, HTACCESS_MAX_ARG_LEN, args, i);
      }
    }

    fclose(fp);
  }
}

int parse_raw_args(char *raw_args, int max_argc, int max_argv_len,
                   char args[][max_argv_len]) {
  char *ptok, *ptokn;
  char *delim = " \t";

  int i = 0;

  ptok = strtok_r(raw_args, delim, &ptokn);

  while (ptok != NULL && i < max_argc) {
    strcpy(args[i], ptok);
    i++;
    ptok = strtok_r(NULL, delim, &ptokn);
  }

  strcpy(args[i], ptokn);

  return i + 1;
}

int parse_htaccess_redirect(session_info *info, int argc, int max_argv_len,
                            char args[][max_argv_len]) {
  if (argc < 3) {
    return -1;
  }

  if (strcmp(args[0], HTACCESS_REDIRECT_PERM) == 0) {
    info->code = 301;
  } else if (strcmp(args[0], HTACCESS_REDIRECT_TEMP) == 0) {
    info->code = 302;
  } else if (strcmp(args[0], HTACCESS_REDIRECT_SEE_OTHER) == 0) {
    info->code = 303;
  }

  // dont use args[1]
  strncpy(info->location, args[2], MAX_PATH_LEN);

  return 0;
}

int parse_htaccess_auth_user_file(session_info *info, int argc,
                                  int max_argv_len, char args[][max_argv_len],
                                  int htaccess_index) {
  if (argc < 1) {
    return -1;
  }

  // convert to relative path from htaccess
  if (args[0][0] != '/') {
    char *path = info->htaccess_paths[htaccess_index];
    char parent[MAX_PATH_LEN];

    get_parent_path(path, parent);
    sprintf(info->auth_user_file, "%s/%s", parent, args[0]);
  } else {
    strncpy(info->auth_user_file, args[0], MAX_PATH_LEN);
  }

  return 0;
}

int parse_htaccess_auth_name(session_info *info, char *raw_args) {
  int i, j;
  // remove "
  for (i = 0; i < strlen(raw_args); i++) {
    if (raw_args[i] == '"') {
      for (j = i; j < strlen(raw_args); j++) {
        raw_args[j] = raw_args[j + 1];
      }
      raw_args[j] = '\0';
    }
  }

  strcpy(info->auth_name, raw_args);

  return 0;
}

int parse_htaccess_auth_type(session_info *info, char *raw_args) {
  if (strcmp(raw_args, "Basic") == 0) {
    info->auth_type = E_AUTH_TYPE_BASIC;
  }

  return 0;
}

int parse_htaccess_errordoc(session_info *info, int argc, int max_argv_len,
                            char args[][max_argv_len], int htaccess_index) {
  if (argc < 2) {
    return -1;
  }

  int code = atoi(args[0]);
  char tmppath[MAX_PATH_LEN + 2];

  // convert to relative path from htaccess
  if (args[1][0] != '/') {
    char *path = info->htaccess_paths[htaccess_index];
    char parent[MAX_PATH_LEN];
    get_parent_path(path, parent);

    sprintf(tmppath, "%s/%s", parent, args[1]);
  } else {
    sprintf(tmppath, "html%s", args[1]);
  }

  switch (code) {
  case 401:
    strcpy(info->doc_401, tmppath);
    break;

  case 403:
    strcpy(info->doc_403, tmppath);
    break;

  case 404:
    strcpy(info->doc_404, tmppath);
    break;

  default:
    return -1;
  }

  return 0;
}
