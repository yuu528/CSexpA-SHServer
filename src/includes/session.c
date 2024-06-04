#include "session.h"
#include "auth.h"
#include "parser.h"
#include "send.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#define RECV_BUF_SIZE 2048

int http_session(int sock) {
  char buf[RECV_BUF_SIZE];
  int recv_size = 0;
  session_info info;
  int ret = 0;

  while (ret == 0) {
    int size = recv(sock, buf + recv_size, RECV_BUF_SIZE, 0);

    if (size == -1) {
      return -1;
    }

    recv_size += size;
    ret = parse_header(buf, recv_size, &info);
  }

  http_reply(sock, &info);

  return 0;
}

void http_reply(int sock, session_info *info) {

  if (info->auth_type == E_AUTH_TYPE_BASIC) {
    if (basic_auth(info->client_authorization, info->auth_user_file) == -1) {
      info->code = 401;
    }
  }

  switch (info->code) {
  case 200:
    send_200(sock, info);
    send_file(sock, info->real_path);
    break;

  case 301:
  case 302:
  case 303:
    send_30x(sock, info->code, info->location);
    printf("%d redirect %s -> %s\n", info->code, info->path, info->location);
    break;

  case 401:
    send_401(sock, info->auth_name);
    printf("401 auth required %s\n", info->path);

    if (access(info->doc_401, F_OK) != -1) {
      send_file(sock, info->doc_401);
    }
    break;

  case 404:
    send_404(sock);
    printf("404 not found %s\n", info->path);

    if (access(info->doc_404, F_OK) != -1) {
      send_file(sock, info->doc_404);
    }
    break;
  }
}
