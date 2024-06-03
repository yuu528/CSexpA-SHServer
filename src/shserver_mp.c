#include "./includes/session.h"
#include "./includes/socketutil.h"

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/wait.h>

void sigChldHandler(int sig) {
  int status = -1;
  pid_t pid = wait(&status);

  printf("child %d terminated\n", pid);
}

int main(int argc, char **argv) {
  printf("Simple HTTP Server: multi-process version\n");

  int sock_listen;

  sock_listen = tcp_listen(10028);

  struct sigaction sa;

  sigaction(SIGCHLD, NULL, &sa);
  sa.sa_handler = sigChldHandler;
  sa.sa_flags = SA_NODEFER;
  sigaction(SIGCHLD, &sa, NULL);

  while (1) {
    struct sockaddr_storage from;
    socklen_t len = sizeof(from);
    int sock_client = 0;

    if ((sock_client = accept(sock_listen, (struct sockaddr *)&from, &len)) ==
        -1) {
      if (errno != EINTR) {
        perror("accept");
      }
    } else {
      char hbus[NI_MAXHOST], sbuf[NI_MAXSERV];

      getnameinfo((struct sockaddr *)&from, len, hbus, sizeof(hbus), sbuf,
                  sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

      pid_t pid = fork();

      if (pid == -1) {
        perror("fork");
        close(sock_client);
      } else if (pid == 0) {
        close(sock_listen);

        http_session(sock_client);
        shutdown(sock_client, SHUT_RDWR);
        close(sock_client);

        sock_client = -1;

        _exit(1);
      } else {
        close(sock_client);
        sock_client = -1;
      }

      int status = -1;
      while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
      }
    }
  }
}
