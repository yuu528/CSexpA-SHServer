#ifndef _H_PARSER_

#define _H_PARSER_

#include "session.h"

#define HEADER_MAX_LEN_PER_LINE 512
#define HEADER_MAX_ARGS 10

#define HEADER_FIELD_AUTHORIZATION "Authorization"
#define HEADER_FIELD_CONTENT_TYPE "Content-Type"
#define HEADER_FIELD_CONTENT_LENGTH "Content-Length"

#define HEADER_KEYWORD_BASIC "Basic"

int parse_header(char *, int, session_info *);
void parse_status(char *, session_info *);
void parse_path_query(char *, session_info *);
void parse_header_field(char *, session_info *);

#endif
