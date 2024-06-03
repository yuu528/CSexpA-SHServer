#ifndef _H_PARSER_

#define _H_PARSER_

#include "session.h"

int parse_header(char *, int, session_info *);
void parse_status(char *, session_info *);
void parse_htaccess(session_info *);

#endif
