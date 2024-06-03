#ifndef _H_SEND_

#define _H_SEND_

void send_30x(int sock, int code, char *location);
void send_401(int, char *);
void send_404(int);
void send_file(int, char *);

#endif
