#include "./includes/session.h"
#include "./includes/socketutil.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>

#define MAX_CHILD 1200

int main(int argc, char **argv) {
  printf("Simple HTTP Server: select version\n");

  int sock_listen;

  int childNum = 0;
  int child[MAX_CHILD];
  for (int i = 0; i < MAX_CHILD; i++) {
    child[i] = -1;
  }

  sock_listen = tcp_listen(10028);

  while (1) {
    fd_set mask;
    FD_ZERO(&mask);
    FD_SET(sock_listen, &mask);
    int width = sock_listen + 1;
    for (int i = 0; i < childNum; i++) {
      if (child[i] != -1) {
        FD_SET(child[i], &mask);
        if (width <= child[i]) {
          width = child[i] + 1;
        }
      }
    }

    fd_set ready = mask;

    struct timeval timeout;
    timeout.tv_sec = 600;
    timeout.tv_usec = 0;

    switch (select(width, (fd_set *)&ready, NULL, NULL, &timeout)) {
    case -1: // error
      perror("select");
      break;

    case 0: // timeout
      break;

    default: // ready
      if (FD_ISSET(sock_listen, &ready)) {
        // Listen socket is ready
        struct sockaddr_storage from;
        socklen_t len = sizeof(from);
        int sock_client = 0;
        if ((sock_client =
                 accept(sock_listen, (struct sockaddr *)&from, &len)) == -1) {
          if (errno != EINTR) {
            perror("accept");
          }
        } else {
          // client
          char hbuf[NI_MAXHOST];
          char sbuf[NI_MAXSERV];
          getnameinfo((struct sockaddr *)&from, len, hbuf, sizeof(hbuf), sbuf,
                      sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
          fprintf(stderr, "accept:%s:%s\n", hbuf, sbuf);

          int pos = -1;
          for (int i = 0; i < childNum; i++) {
            if (child[i] == -1) {
              pos = i;
              break;
            }
          }

          if (pos == -1) {
            if (childNum >= MAX_CHILD) {
              fprintf(stderr, "child is full.\n");
              close(sock_client);
            } else {
              pos = childNum;
              childNum = childNum + 1;
            }
          }

          if (pos != -1) {
            child[pos] = sock_client;
          }
        }
      }

      for (int i = 0; i < childNum; i++) {
        if (child[i] != -1 && FD_ISSET(child[i], &ready)) {
          http_session(child[i]);
          close(child[i]);
          child[i] = -1;
        }
      }
    }
  }
}
