#include "parser.h"
#include "fileutil.h"

#include <stdio.h>
#include <string.h>

#define HEADER_MAX_LEN_PER_LINE 128

int parse_header(char *buf, int size, session_info *info) {
  char status[HEADER_MAX_LEN_PER_LINE];
  int i, j;

  enum state_type { PARSE_STATUS, PARSE_OTHERS, PARSE_END } state;

  state = PARSE_STATUS;
  j = 0;

  for (i = 0; i < size; i++) {
    switch (state) {
    case PARSE_STATUS:
      if (buf[i] == '\r') {
        status[j] = '\0';
        j = 0;
        state = PARSE_END;
        parse_status(status, info);
        check_file(info);
        find_htaccess(info);
        parse_htaccess(info);
      } else {
        status[j] = buf[i];
        j++;
      }
      break;
    }

    if (state == PARSE_END) {
      return 1;
    }
  }

  return 0;
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

void parse_htaccess(session_info *info) {
  FILE *fp;
  char buf[1024];
  char args[5][1024];

  char *delim = " \t";
  char *ptok;

  int i, j = 0;
  int argc;
  int ret;

  for (i = info->htaccess_count - 1; i >= 0; i--) {
    fp = fopen(info->htaccess_paths[i], "r");

    if (fp == NULL) {
      continue;
    }

    while (fgets(buf, 1024, fp) != NULL) {
      if (buf[0] == '#') {
        continue;
      }

      ptok = strtok(buf, delim);

      while (ptok != NULL) {
        strcpy(args[j], ptok);

        j++;
        ptok = strtok(NULL, delim);
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
      }
    }

    fclose(fp);
  }
}
