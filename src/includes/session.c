#include "session.h"
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

  if (info->code == 404) {
    send_404(sock);
    printf("404 not found %s\n", info->path);
    return;
  } else if (301 <= info->code && info->code <= 303) {
    char msg[1024];

    switch (info->code) {
    case 301:
      strcpy(msg, "HTTP/1.0 301 Moved Permanently\r\n");
      break;

    case 302:
      strcpy(msg, "HTTP/1.0 302 Found\r\n");
      break;

    case 303:
      strcpy(msg, "HTTP/1.0 303 See Other\r\n");
      break;
    }

    sprintf(buf, "%sLocation: %s\r\n", msg, info->location);

    printf("redirect %s -> %s\n", info->path, info->location);

    ret = send(sock, buf, strlen(buf), MSG_NOSIGNAL);

    if (ret < 0) {
      shutdown(sock, SHUT_RDWR);
      close(sock);
    }
    return;
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
