#include "send.h"
#include "fileutil.h"

#include <stdio.h>
#include <stdlib.h>
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

void send_file(int sock, char *filename, int additional_header) {
  FILE *fp;
  int len;
  char buf[FILE_BUFFER_SIZE];
  char *header_end_pos;

  int size = get_file_size(filename);
  char type[32];

  // send header
  static const int header_base_len = sizeof(HEADER_CONTENT_FORMAT);

  char header_buf[header_base_len + count_digits(size) + strlen(type)];

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return;
  }

  if (additional_header == 1) {
    // sub input file header length from len
    while ((len = fread(buf, sizeof(char), FILE_BUFFER_SIZE, fp)) > 0) {
      header_end_pos = strstr(buf, CRLF CRLF);

      if (header_end_pos != NULL) {
        size -= header_end_pos - buf + 4;
        break;
      }

      size -= len;
    }

    rewind(fp);

    sprintf(header_buf, HEADER_CONTENT_LENGTH "%d" CRLF, size);
  } else {
    get_file_type(filename, type);
    sprintf(header_buf, HEADER_CONTENT_FORMAT CRLF, size, type);
  }

  send_http_msg(sock, header_buf);

  // send file

  len = fread(buf, sizeof(char), FILE_BUFFER_SIZE, fp);
  while (len > 0) {
    int ret = send(sock, buf, len, MSG_NOSIGNAL);
    if (ret < 0) {
      shutdown(sock, SHUT_RDWR);
      close(sock);
      break;
    }
    len = fread(buf, sizeof(char), FILE_BUFFER_SIZE, fp);
  }

  fclose(fp);
}

void send_file_cgi(int sock, char *filename) {
  char tmpfile[L_tmpnam];
  char *pext = strrchr(filename, '.');

  char cmd[sizeof(CGI_CMD_PHP) + strlen(filename)];

  tmpnam(tmpfile);

  if (pext != NULL) {
    if (strcmp(pext, EXT_PHP) == 0) {
      sprintf(cmd, CGI_CMD_PHP, filename, tmpfile);
      system(cmd);

      send_file(sock, tmpfile, 1);

      return;
    } else if (strcmp(pext, EXT_CGI) == 0) {
      sprintf(cmd, CGI_CMD_CGI, filename, tmpfile);
      system(cmd);

      send_file(sock, tmpfile, 1);

      return;
    }
  }

  send_file(sock, filename, 0);
}
