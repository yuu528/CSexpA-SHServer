#include "benchlib.h"
#include "exp1.h"

#include <sys/resource.h>

void randomize_array(int arr[], int size) {
  srand(time(NULL));
  for (int i = 0; i < size; i++) {
    int temp = arr[i];
    int r = rand() % size;
    arr[i] = arr[r];
    arr[r] = temp;
  }
}

void update_rlimit(int resource, int soft, int hard) {
  struct rlimit rl;
  getrlimit(resource, &rl);
  rl.rlim_cur = soft;
  rl.rlim_max = hard;
  if (setrlimit(resource, &rl) == -1) {
    perror("setrlimit");
    exit(-1);
  }
}
