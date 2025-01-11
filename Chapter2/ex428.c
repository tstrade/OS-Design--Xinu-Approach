#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int n = 0;

void produce() {
  int i;
  for (i = 1; i <= 2000; i++) {
    n++;
  }
}

void consume() {
  int i;
  for (i = 1; i <= 2000; i++) {
    printf("The value of n is %d\n", n);
  }
}

int main() {
  pid_t pA = fork();
  if (pA < 0) {
    perror("Fork A failed\n");
  } else if (pA == 0) {
    produce();
  }

  pid_t pB = fork();
  if (pB < 0) {
    perror("For B failed\n");
  } else if (pB == 0) {
    consume();
  }

  return 0;
}
