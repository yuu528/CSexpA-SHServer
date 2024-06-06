#include "send.h"
#include "fileutil.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

int count_digits(int n) {
  int count = 0;

  while (n != 0) {
    n /= 10;
    count++;
  }

  return count;
}

void send_http_msg(int sock, char *msg) {
  int ret = send(sock, msg, strlen(msg), MSG_NOSIGNAL);

  if (ret < 0) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
  }
}

void send_200(int sock) { send_http_msg(sock, HTTP_200_FORMAT); }

void send_30x(int sock, int code, char *location) {
  // max length: 301 format
  static const int base_len =
      sizeof(HTTP_30X_FORMAT(301) HEADER_LOCATION_FORMAT);
  char buf[base_len + strlen(location)];

  int len;

  switch (code) {
  case 301:
    len = sprintf(buf, HTTP_30X_FORMAT(301));
    break;

  case 302:
    len = sprintf(buf, HTTP_30X_FORMAT(302));
    break;

  case 303:
    len = sprintf(buf, HTTP_30X_FORMAT(303));
    break;
  }

  sprintf(buf + len, HEADER_LOCATION_FORMAT, location);

  send_http_msg(sock, buf);
}

void send_401(int sock, char *realm) {
  static const int base_len = sizeof(HTTP_401_FORMAT HEADER_WWW_AUTH_FORMAT);
  char buf[base_len + strlen(realm)];

  sprintf(buf, HTTP_401_FORMAT HEADER_WWW_AUTH_FORMAT, realm);

  send_http_msg(sock, buf);
}

void send_404(int sock) { send_http_msg(sock, HTTP_404_FORMAT); }

void send_file(int sock, char *filename) {
  FILE *fp;
  int len;
  char buf[16384];

  int size = get_file_size(filename);
  char type[32];

  get_file_type(filename, type);

  // send header
  static const int header_base_len = sizeof(HEADER_CONTENT_FORMAT);

  char header_buf[header_base_len + count_digits(size) + strlen(type)];

  sprintf(header_buf, HEADER_CONTENT_FORMAT CRLF, size, type);

  send_http_msg(sock, header_buf);

  // send file
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
