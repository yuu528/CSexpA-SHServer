#ifndef _H_SEND_

#define _H_SEND_

#include "session.h"

#define HTTP_VERSION "HTTP/1.1 "

#define HTTP_200 "200 OK"

#define HTTP_301 "301 Moved Permanently"
#define HTTP_302 "302 Found"
#define HTTP_303 "303 See Other"

#define HTTP_401 "401 Unauthorized"
#define HTTP_403 "403 Forbidden"
#define HTTP_404 "404 Not Found"

#define HEADER_CONTENT_LENGTH "Content-Length: "
#define HEADER_CONTENT_TYPE "Content-Type: "

#define HEADER_LOCATION "Location: "

#define HEADER_WWW_AUTH "WWW-Authenticate: "
#define HEADER_WWW_AUTH_BASIC "Basic"
#define HEADER_WWW_AUTH_REALM " realm=\"%s\""

#define CRLF "\r\n"

#define HTTP_200_FORMAT HTTP_VERSION HTTP_200 CRLF
#define HTTP_FORMAT(code) HTTP_VERSION HTTP_##code CRLF

#define HEADER_CONTENT_FORMAT                                                  \
  HEADER_CONTENT_LENGTH "%d" CRLF HEADER_CONTENT_TYPE "%s" CRLF

#define HEADER_LOCATION_FORMAT HEADER_LOCATION "%s" CRLF

#define HEADER_WWW_AUTH_FORMAT                                                 \
  HEADER_WWW_AUTH HEADER_WWW_AUTH_BASIC HEADER_WWW_AUTH_REALM CRLF

#define CGI_VERSION "CGI/1.1"

#define CGI_ENV(name) " " #name "="

#define CGI_CMD_PHP_FORMAT " php-cgi %s > %s"
#define CGI_CMD_CGI_FORMAT " %s > %s"

int count_digits(int);
void send_http_msg(int, char *);
void send_200(int);
void send_30x(int, int, char *);
void send_401(int, char *);
void send_403(int);
void send_404(int);
void send_file(int, char *, int);
void send_file_cgi(int, char *);

#endif
