#include "exp1.h"
#include "exp1lib.h"

#include "benchlib.h"

#include <sys/resource.h>

#define DEFAULT_PORT 10028

const char *base_path = "";

char g_hostname[256];
pthread_mutex_t g_mutex;
int g_error_count;

int port = DEFAULT_PORT;

void *exp1_eval_thread(void *param);
void exp1_session_error();

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("usage: %s <ip address> <# of clients> [port]\n", argv[0]);
    exit(-1);
  }

  if (argc == 4) {
    port = atoi(argv[3]);
  }

  strcpy(g_hostname, argv[1]);
  int th_num = atoi(argv[2]);

  int fileid_arr_size = 100;
  int fileid_arr[fileid_arr_size];

  for (int i = 0; i < fileid_arr_size; i++) {
    fileid_arr[i] = i;
  }

  randomize_array(fileid_arr, fileid_arr_size);

  // set resource to unlimited
  update_rlimit(RLIMIT_STACK, (int)RLIM_INFINITY, (int)RLIM_INFINITY);

  g_error_count = 0;
  pthread_mutex_init(&g_mutex, NULL);
  pthread_t *th = malloc(sizeof(pthread_t) * th_num);

  struct timespec ts_start;
  struct timespec ts_end;
  clock_gettime(CLOCK_MONOTONIC, &ts_start);

  for (int i = 0; i < th_num; i++) {
    int *pfileid = malloc(sizeof(int));
    *pfileid = fileid_arr[i % fileid_arr_size];
    pthread_create(&th[i], NULL, exp1_eval_thread, pfileid);
  }

  for (int i = 0; i < th_num; i++) {
    pthread_join(th[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  time_t diffsec = difftime(ts_end.tv_sec, ts_start.tv_sec);
  long diffnanosec = ts_end.tv_nsec - ts_start.tv_nsec;
  double total_time = diffsec + diffnanosec * 1e-9;

  printf("total time is %10.10f\n", total_time);
  printf("session error ratio is %1.3f\n",
         (double)g_error_count / (double)th_num);

  free(th);
}

void *exp1_eval_thread(void *param) {
  int sock;
  int *pfileid;
  int fileid;
  char command[1024];
  char buf[2048];
  int total;
  int ret;

  pfileid = (int *)param;
  fileid = *pfileid;
  free(pfileid);

  sock = exp1_tcp_connect(g_hostname, port);
  if (sock < 0) {
    exp1_session_error();
    pthread_exit(NULL);
  }

  sprintf(command, "GET %s/%03d.jpg HTTP/1.0\r\n\r\n", base_path, fileid);
  ret = send(sock, command, strlen(command), 0);
  if (ret < 0) {
    exp1_session_error();
    pthread_exit(NULL);
  }

  total = 0;
  ret = recv(sock, buf, 2048, 0);

  char res_buf[2048];
  strcpy(res_buf, buf);
  char *res_code = strtok(res_buf, " ");
  res_code = strtok(NULL, " ");

  while (ret > 0) {
    total += ret;
    ret = recv(sock, buf, 2048, 0);
  }
  fprintf(stderr, "GET %s/%03d.jpg HTTP/1.0", base_path, fileid);
  fprintf(stderr, " -> Response Code: %s: Recieved Size: %d bytes\n", res_code,
          total);

  pthread_exit(NULL);
}

void exp1_session_error() {
  pthread_mutex_lock(&g_mutex);
  g_error_count++;
  pthread_mutex_unlock(&g_mutex);

  return;
}
