#include "send.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

void send_30x(int sock, int code, char *location) {
  char buf[16384];
  char msg[128];
  int ret;

  switch (code) {
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

  sprintf(buf, "%sLocation: %s\r\n", msg, location);

  ret = send(sock, buf, strlen(buf), MSG_NOSIGNAL);

  if (ret < 0) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
  }
  return;
}

void send_401(int sock, char *realm) {
  char buf[16384];
  int ret;

  sprintf(buf, "HTTP/1.0 401 Authorization Required\r\n");
  sprintf(buf + strlen(buf), "WWW-Authenticate: Basic realm=\"%s\"\r\n", realm);

  ret = send(sock, buf, strlen(buf), MSG_NOSIGNAL);

  if (ret < 0) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
  }
}

void send_404(int sock) {
  char buf[16384];
  int ret;

  sprintf(buf, "HTTP/1.0 404 Not Found\r\n\r\n");
  printf("%s", buf);
  ret = send(sock, buf, strlen(buf), MSG_NOSIGNAL);

  if (ret < 0) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
  }
}

void send_file(int sock, char *filename) {
  FILE *fp;
  int len;
  char buf[16384];

  fp = fopen(filename, "r");
  if (fp == NULL) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return;
  }

  len = fread(buf, sizeof(char), 16384, fp);
  while (len > 0) {
    int ret = send(sock, buf, len, MSG_NOSIGNAL);
    if (ret < 0) {
      shutdown(sock, SHUT_RDWR);
      close(sock);
      break;
    }
    len = fread(buf, sizeof(char), 1460, fp);
  }

  fclose(fp);
}
