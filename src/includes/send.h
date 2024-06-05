#ifndef _H_SEND_

#define _H_SEND_

#include "session.h"

void send_http_msg(int, char *);
void send_200(int, session_info *);
void send_30x(int, int, char *);
void send_401(int, char *);
void send_404(int);
void send_file(int, char *);

#endif
