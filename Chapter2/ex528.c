#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

int n = 0;

void produce(sem_t *producer, sem_t *consumer) {
  int i;
  for (i = 1; i <= 2000; i++) {
    sem_wait(consumer);
    n++;
    sem_post(producer);
  }
}

void consume(sem_t *producer, sem_t *consumer) {
  int i;
  for (i = 1; i <= 2000; i++) {
    sem_wait(producer);
    printf("The value of n is %d\n", n);
    sem_post(consumer);
  }
}

int main() {
  sem_t producer, consumer;
  sem_init(&producer, 1, 1);
  sem_init(&consumer, 1, 1);

  pid_t pA = fork();
  if (pA < 0) {
    perror("Fork A failed\n");
  } else if (pA == 0) {
    produce(&producer, &consumer);
  }

  consume(&producer, &consumer);

  return 0;
}
