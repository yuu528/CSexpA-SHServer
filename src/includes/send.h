#ifndef _H_SEND_

#define _H_SEND_

#include "session.h"

void send_200(int sock, session_info *);
void send_30x(int sock, int code, char *location);
void send_401(int, char *);
void send_404(int);
void send_file(int, char *);

#endif
