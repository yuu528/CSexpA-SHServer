#include "./includes/session.h"
#include "./includes/socketutil.h"

#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>

int main(int argc, char **argv) {
  printf("Simple HTTP Server: Non-multiplexing version\n");

  int sock_listen;

  sock_listen = tcp_listen(10028);

  while (1) {
    struct sockaddr addr;
    int sock_client;
    int len;

    sock_client = accept(sock_listen, &addr, (socklen_t *)&len);
    http_session(sock_client);

    shutdown(sock_client, SHUT_RDWR);
    close(sock_client);
  }
}
