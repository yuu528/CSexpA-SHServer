#include "fileutil.h"
#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

int get_parent_path(char *path, char *parent) {
  char *p;
  strcpy(parent, path);
  p = strrchr(parent, '/');
  if (p != NULL) {
    *p = '\0';
    return 0;
  } else {
    return -1;
  }
}

void check_file(session_info *info) {
  struct stat s;
  int ret;
  char *pext;

  sprintf(info->real_path, "html%s", info->path);
  ret = stat(info->real_path, &s);

  if ((s.st_mode & S_IFMT) == S_IFDIR) {
    sprintf(info->real_path, "%s/index.html", info->real_path);
  }

  ret = stat(info->real_path, &s);

  if (ret == -1) {
    info->code = 404;
  } else {
    info->code = 200;
    info->size = (int)s.st_size;
  }

  pext = strstr(info->real_path, ".");

  if (pext != NULL) {
    if (strcmp(pext, ".html") == 0) {
      strcpy(info->type, "text/html");
    } else if (strcmp(pext, ".jpg") == 0) {
      strcpy(info->type, "image/jpeg");
    } else if (strcmp(pext, ".png") == 0) {
      strcpy(info->type, "image/png");
    } else if (strcmp(pext, ".gif") == 0) {
      strcpy(info->type, "image/gif");
    }
  }
}

void find_htaccess(session_info *info) {
  char cur_path[MAX_PATH_LEN], next_path[MAX_PATH_LEN];
  int ret;
  int htaccess_idx = 0;

  strcpy(cur_path, info->real_path);

  do {
    if (get_parent_path(cur_path, next_path) == -1) {
      break;
    }

    strcpy(cur_path, next_path);

    sprintf(info->htaccess_paths[htaccess_idx], "%s/.htaccess", cur_path);

    if (access(info->htaccess_paths[htaccess_idx], R_OK) == 0) {
      htaccess_idx++;
    }
  } while (htaccess_idx < HTACCESS_MAX);

  info->htaccess_count = htaccess_idx;
}
