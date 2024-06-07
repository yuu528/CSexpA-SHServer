#ifndef _H_PARSER_

#define _H_PARSER_

#include "session.h"

#define HEADER_MAX_LEN_PER_LINE 128
#define HEADER_MAX_ARGS 10

int parse_header(char *, int, session_info *);
void parse_status(char *, session_info *);
void parse_path_query(char *, session_info *);
void parse_header_field(char *, session_info *);

#endif
