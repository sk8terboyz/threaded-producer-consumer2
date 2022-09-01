#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>

#define RAND_MAX 10
#define BUFFER_SIZE 5

/* data shared between threads */
char *buffer[BUFFER_SIZE];
int counter = 0;
sem_t sem;
pthread_mutex_t lock;
char **buf;

void *insert_item()
{

  int in = 0;
  bool forever = true;
  while(forever)
  {
    int sleeping = rand() % RAND_MAX;
    sem_wait(&sem);
    pthread_mutex_lock(&lock);
    sleep(sleeping);
    while(counter == BUFFER_SIZE);
    int item = rand() % sizeof(buf);  //find value to insert into buffer
    buffer[in] = buf[item];
    printf("\n[producer thread ID: %lu] inserted an item (word: %s) to the buffer\n", pthread_self(), buffer[in]);
    in = (in + 1) % BUFFER_SIZE;
    counter++;
    pthread_mutex_unlock(&lock);
    sem_post(&sem);
  }
}

void *remove_item()
{
  int out = 0;
  bool forever = true;;
  while(forever)
  {
    char* output;
    int sleeping = rand() % RAND_MAX;
    sem_wait(&sem);
    pthread_mutex_lock(&lock);
    sleep(sleeping);
    while(counter == 0);
    output = buffer[out];
    printf("[consumer thread ID: %lu] removed an item (word: %s) from the buffer\n", pthread_self(), buffer[out]);
    out = (out + 1) % BUFFER_SIZE;
    counter--;
    pthread_mutex_unlock(&lock);
    sem_post(&sem);
  }
}

int main(int argc, char *argv[])
{
  int sleepTime;
  int producers;
  int consumers;
  /* store command line arguments */
  if(argc == 4)
  {
    sleepTime = atoi(argv[1]);
    producers = atoi(argv[2]);
    consumers = atoi(argv[3]);
  }
  else
  {
    /* print out error message for improper usage */
    printf("Proper Usage : ./producer_consumer2 [runtime] [# of producers] [# of consumers]\n");
    exit(-1);
  }

  /* read in the entire text file into the array */
  FILE *source;
  char* reader;

  source = fopen("wordsEn.txt", "r");
  if(source == NULL)
  {
    fprintf(stderr, "File could not be opened\n");
    exit(1);
  }

  fseek(source, 0, SEEK_END);
  int size = ftell(source);
  rewind(source);

  reader = (char*)malloc(sizeof(char)*size);
  if(!reader)
  {
    printf("Failed to malloc\n");
    exit(1);
  }

  buf = (char**)malloc(size*sizeof(char*));
  for(int i = 0; i < size/2; i++)
  {
    buf[i] = (char*)malloc((size/2)*sizeof(char));
  }
  if(!buf)
  {
    printf("Failed to malloc\n");
    exit(1);
  }

  /* read in the entire file to reader array */
  int checkr = fread(reader, sizeof(char), size, source);

  int i = 0;
  int k = 0;
  int j = 0;
  while(reader[k] != '\0')
  {
    if(reader[k] == ' ' || reader[k] == '\n')
    {
      k++;
      while(reader[k] == ' ')
      {
        k++;
      }
      i++;
      j = 0;
    }
    if(reader[k] != ' ' || reader[k] != '\n')
    {
      buf[i][j] = reader[k];
      k++;
      j++;
    }
  }

  fclose(source);

  /* initialize semaphore */
  sem_init(&sem, 0, 1);

  /* initialize mutex lock */
  if(pthread_mutex_init(&lock, NULL) != 0)
  {
    printf("mutex init failed\n");
    exit(-1);
  }

  /* create the producer threads */
  pthread_t thread_id1[producers];
  for(int i = 0; i < producers; i++)
  {
    pthread_create(&thread_id1[i], NULL, &insert_item, NULL);
  }

  /* create the consumer threads */
  pthread_t thread_id2[consumers];
  for(int i = 0; i < consumers; i++)
  {
    pthread_create(&thread_id2[i], NULL, &remove_item, NULL);
  }
  /* main thread sleeps then exits the program */
  sleep(sleepTime);
  sem_destroy(&sem);
  exit(0);

  pthread_mutex_destroy(&lock);

  return 0;
}
