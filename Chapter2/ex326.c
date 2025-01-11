#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void sndch(char c) {
  while (1) {
    putc(c, stdout);
  }
}

int main() {
  char a = 'A', b = 'B';

  pid_t pA = fork();
  if (pA < 0) {
    perror("Fork A failed\n");
  } else if (pA == 0) {
    sndch(a);
  }

  pid_t pB = fork();
  if (pB < 0) {
    perror("Fork B failed\n");
  } else if (pB == 0) {
    sndch(b);
  }

  return 0;
}
