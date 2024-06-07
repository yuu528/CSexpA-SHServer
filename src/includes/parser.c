#include "parser.h"
#include "fileutil.h"
#include "htaccess_parser.h"
#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_header(char *buf, int size, session_info *info) {
  char *ptok, *ptokn;
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
  char argv[HEADER_MAX_LEN_PER_LINE];
  char *ptok, *ptokn;
  char *delim = " ";
  int i = 0;

  strcpy(argv, status);

  ptok = strtok_r(argv, delim, &ptokn);

  while (ptok != NULL && i < 2) {
    if (i == 0) {
      strcpy(pinfo->cmd, ptok);
    } else if (i == 1) {
      parse_path_query(ptok, pinfo);
    }

    i++;

    ptok = strtok_r(NULL, delim, &ptokn);
  }
}

void parse_path_query(char *raw_path, session_info *info) {
  char *ptok, *ptokn;
  char *delim = "?";

  ptok = strtok_r(raw_path, delim, &ptokn);

  strcpy(info->path, ptok);

  if (ptokn != NULL) {
    strcpy(info->query, ptokn);
  } else {
    strcpy(info->query, "");
  }
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
