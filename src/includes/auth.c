#include "auth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int base64_decode(char *input, char *output) {
  char tmpfile[L_tmpnam];
  char cmd[1024];

  tmpnam(tmpfile);

  sprintf(cmd, "echo '%s' | base64 -d > %s", input, tmpfile);
  system(cmd);

  FILE *fp = fopen(tmpfile, "r");
  if (fp == NULL) {
    return -1;
  }

  fgets(output, 1024, fp);

  return 0;
}

int basic_auth(char *cred, char *path) {
  FILE *fp;

  char line[1024];
  char decoded[1024];
  char *ptok, *ptokn;

  if (strcmp(cred, "") == 0) {
    return -1;
  }

  if (base64_decode(cred, decoded) == -1) {
    return -1;
  }

  // check user and password in htpaswd
  fp = fopen(path, "r");
  if (fp == NULL) {
    return -1;
  }

  while (fgets(line, sizeof(line), fp) != NULL) {
    line[strlen(line) - 1] = '\0';

    if (strcmp(line, decoded) == 0) {
      fclose(fp);
      return 0;
    }
  }

  fclose(fp);
  return -1;
}
