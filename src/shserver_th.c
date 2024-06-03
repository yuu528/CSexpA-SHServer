#include "./includes/session.h"
#include "./includes/socketutil.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>

void *exp1_thread(void *param) {
  int *psock;
  int sock;

  pthread_detach(pthread_self());
  psock = (int *)param;
  sock = *psock;
  free(psock);

  http_session(sock);

  close(sock);
  shutdown(sock, SHUT_RDWR);

  printf("thread %d is ending\n", sock);

  return NULL;
}

void exp1_create_thread(int sock) {
  int *psock;
  pthread_t th;
  psock = malloc(sizeof(int));
  *psock = sock;

  if (pthread_create(&th, NULL, exp1_thread, psock) != 0) {
    printf("pthread %d is failed\n", sock);
    close(sock);
  }

  printf("thread %d is created\n", sock);
}

int main(int argc, char **argv) {
  printf("Simple HTTP Server: pthread version\n");

  int sock_listen;

  sock_listen = tcp_listen(10028);

  while (1) {
    struct sockaddr addr;
    int sock_client;
    int len;

    if ((sock_client = accept(sock_listen, &addr, (socklen_t *)&len)) == -1) {
      if (errno != EINTR) {
        perror("accept");
      }
    }

    exp1_create_thread(sock_client);
  }
}
