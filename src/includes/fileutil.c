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

int get_file_size(char *path) {
  struct stat s;

  if (stat(path, &s) == -1) {
    return -1;
  } else {
    return (int)s.st_size;
  }
}

void get_file_type(char *path, char *type) {
  char *pext = strrchr(path, '.');

  if (pext != NULL) {
    if (strcmp(pext, EXT_HTML) == 0 || strcmp(pext, EXT_HTM) == 0 ||
        strcmp(pext, EXT_PHP) == 0) {
      strcpy(type, MIME_HTML);
    } else if (strcmp(pext, EXT_JPG) == 0) {
      strcpy(type, MIME_JPG);
    } else if (strcmp(pext, EXT_PNG) == 0) {
      strcpy(type, MIME_PNG);
    } else if (strcmp(pext, EXT_GIF) == 0) {
      strcpy(type, MIME_GIF);
    } else {
      strcpy(type, MIME_TXT);
    }
  } else {
    strcpy(type, MIME_TXT);
  }
}

void check_file(session_info *info) {
  struct stat s;
  int ret, i = 0;
  char real_path[MAX_PATH_LEN + 1];
  char *pext;

  static const int index_file_count = 3;
  static const char *index_file[] = {"index.html", "index.htm", "index.php"};

  sprintf(info->real_path, "html%s", info->path);
  ret = stat(info->real_path, &s);

  if ((s.st_mode & S_IFMT) == S_IFDIR) {
    do {
      sprintf(real_path, "%s/%s", info->real_path, index_file[i++]);
    } while (access(real_path, F_OK) == -1 && i < index_file_count);

    strcpy(info->real_path, real_path);
  }

  ret = stat(info->real_path, &s);

  if (ret == -1 || access(info->real_path, F_OK) == -1) {
    info->code = 404;
  } else if (access(info->real_path, R_OK) == 0) {
    info->code = 200;
    info->size = (int)s.st_size;

    pext = strrchr(info->real_path, '.');
    if (pext != NULL) {
      if ((strcmp(pext, EXT_CGI) == 0) && access(info->real_path, X_OK) == -1) {
        info->code = 403;
      }
    }
  } else {
    info->code = 403;
  }

  get_file_type(info->real_path, info->type);
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
