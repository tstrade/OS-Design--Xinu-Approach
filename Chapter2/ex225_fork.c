#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void sndA(void) {
  while (1) {
    putc('A', stdout);
  }
}

void sndB(void) {
  while (1) {
    putc('B', stdout);
  }
}

int main() {
  pid_t pA = fork();
  if (pA < 0) {
    perror("Fork A failed\n");
  } else if (pA == 0) {
    sndA();
  }

  pid_t pB = fork();
  if (pB < 0) {
    perror("Fork B failed\n");
  } else if (pB == 0) {
    sndB();
  }

  return 0;
}
