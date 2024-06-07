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
  info->client_size = -1;
  strcpy(info->client_authorization, "");
  strcpy(info->content, "");
  strcpy(info->client_type, "");

  ptok = strtok_r(buf, delim, &ptokn);

  while (ptok != NULL) {
    // remove \r
    for (int j = 0; j < strlen(ptok); j++) {
      if (ptok[j] == '\r') {
        ptok[j] = '\0';
      }
    }

    if (i == 0) {
      parse_status(ptok, info);
    } else {
      if (strlen(ptok) > 0) {
        parse_header_field(ptok, info);
      } else if (strlen(ptokn) > 0) {
        strcpy(info->content, ptokn);
        break;
      }
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
  char *ptok, *ptokn, *ptok2, *ptokn2, *argv;
  char *delim = ":", *delim2 = " ";
  int i = 0, j;

  // remove \r
  for (int k = 0; k < strlen(line); k++) {
    if (line[k] == '\r' || line[k] == '\n') {
      line[k] = '\0';
    }
  }

  ptok = strtok_r(line, delim, &ptokn);

  while (ptok != NULL) {
    strcpy(args[i], ptok);

    ptok = strtok_r(NULL, delim, &ptokn);

    i++;
  }

  // skip space
  j = 0;
  while (args[1][j] == ' ') {
    j++;
  }
  argv = args[1] + j;

  // parse args
  if (strcmp(args[0], HEADER_FIELD_AUTHORIZATION) == 0) {
    ptok2 = strtok_r(argv, delim2, &ptokn2);

    // check auth type
    if (strcmp(argv, HEADER_KEYWORD_BASIC) == 0) {
      strcpy(info->client_authorization, ptokn2);
    }
  } else if (strcmp(args[0], HEADER_FIELD_CONTENT_TYPE) == 0) {
    strcpy(info->client_type, argv);
  } else if (strcmp(args[0], HEADER_FIELD_CONTENT_LENGTH) == 0) {
    info->client_size = atoi(argv);
  }
}
