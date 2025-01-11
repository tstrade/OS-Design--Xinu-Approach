#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

pthread_t p[2];

void *sndA(void *arg) {
  if (arg != NULL) { exit(EXIT_FAILURE); }

  while (1) {
    putc('A', stdout);
  }
  pthread_exit(NULL);
}

void *sndB(void *arg) {
  if (arg != NULL) { exit(EXIT_FAILURE); }

  while (1) {
    putc('B', stdout);
  }
}

int main() {

  void (*func_ptr[2]) = { sndA, sndB };

  for (int i = 0; i < 2; i++) {
    pthread_create(&p[i], NULL, func_ptr[i], NULL);
  }

  for (int j = 0; j < 2; j++) {
    pthread_join(p[j], NULL);
  }

  pthread_exit(NULL);
  return 0;
}
