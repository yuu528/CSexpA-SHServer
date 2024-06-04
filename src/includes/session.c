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
  char buf[16384];
  int len;
  int ret;

  switch (info->code) {
  case 301:
  case 302:
  case 303:
    send_30x(sock, info->code, info->location);
    printf("%d redirect %s -> %s\n", info->code, info->path, info->location);
    return;

  case 404:
    send_404(sock);
    printf("404 not found %s\n", info->path);
    return;
  }

  if (info->auth_type == E_AUTH_TYPE_BASIC) {
    printf("client authorization: %s\n", info->client_authorization);
    if (basic_auth(info->client_authorization, info->auth_user_file) == -1) {
      send_401(sock, info->auth_name);
      printf("401 auth required %s\n", info->path);
      return;
    }
  }

  len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
  len += sprintf(buf + len, "Content-Length: %d\r\n", info->size);
  len += sprintf(buf + len, "Content-Type: %s\r\n", info->type);
  len += sprintf(buf + len, "\r\n");

  ret = send(sock, buf, len, MSG_NOSIGNAL);
  if (ret < 0) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return;
  }

  send_file(sock, info->real_path);
}
