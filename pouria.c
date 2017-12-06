#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct buf_thread {
    int numTH;
    unsigned long *buffer;
};


int buffer_size=10;
int workers=1;
int LOOPS=10;
static int no_of_read_lines = 0;
static int end_of_producing = 0;
static int counting_producer = 0;
static int counting_dispatcher = 0;
static int worker_position = 0;
static int dispatcher_position = 0;
static int end_of_dispacher = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER, fill = PTHREAD_COND_INITIALIZER;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER, buffer_fill = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_filetw = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {

    srand((unsigned long) time(NULL));

    for (int j = 0; j < LOOPS; j++) {
        pthread_mutex_lock(&mutex);
        FILE *file = fopen("/home/pouria/Desktop/Info.txt", "a");

        int random_num = rand() % 49 + 1;


        //;
        for (int i = 0; i < random_num; i++) {
            time_t now = time(0);
            fprintf(file, "%lu\n", now);
            counting_producer++;
            usleep(50 * 1000);
        }

        fclose(file);
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);


      
        usleep((rand() % 40 + 10) * 1000);

    }
    end_of_producing = 1;
    printf("counting_producer : %d\n", counting_producer);


    return NULL;
}

void *dispatcher(void *arg) {
    ssize_t read;
    size_t len = 0;
    char *line = NULL;

    struct buf_thread myPair = *(struct buf_thread *) arg;

    unsigned long *myBuffer = myPair.buffer;

    sleep(3);

    while (!(end_of_producing == 1 && counting_dispatcher >=
                                        counting_producer)) {
        pthread_mutex_lock(&mutex);
        FILE *file = fopen("/home/pouria/Desktop/Info.txt", "r");


        while (counting_producer < counting_dispatcher)
            pthread_cond_wait(&fill, &mutex);


        pthread_mutex_lock(&buffer_mutex);
        while (dispatcher_position - worker_position > buffer_size)
            pthread_cond_wait(&buffer_empty, &buffer_mutex);

        fseek(file, no_of_read_lines * 11, SEEK_SET);

        read = getline(&line, &len, file);
        myBuffer[dispatcher_position % buffer_size] = atoi(line);
        printf("dispatcher %lu \n", myBuffer[dispatcher_position % buffer_size]);
        pthread_cond_signal(&buffer_fill);
        pthread_mutex_unlock(&buffer_mutex);

        no_of_read_lines++;
        counting_dispatcher++;
        dispatcher_position++;



        fclose(file);

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        usleep(5000);
    }
    printf("counting_dispatcher : %d\n", counting_dispatcher);
    end_of_dispacher=1;
    // pthread_mutex_lock(&buffer_mutex);
    pthread_cond_signal(&buffer_fill);
    // pthread_mutex_unlock(&buffer_mutex);
    return NULL;
}

void *worker(void *arg) {
    sleep(6);
    struct buf_thread myPair = *(struct buf_thread *) arg;


    unsigned long *myBuffer = myPair.buffer;

    while (!(end_of_producing == 1 && counting_dispatcher >=
                                        counting_producer && end_of_dispacher==1)) {
        pthread_mutex_lock(&buffer_mutex);
        if(end_of_dispacher)
                return NULL;
        while (worker_position >= dispatcher_position && !end_of_dispacher){
            if(end_of_dispacher)
              return NULL;
            //printf("i'll wait\n" );
            pthread_cond_wait(&buffer_fill, &buffer_mutex);}

        //printf("worker num %d\n",myPair.numTH );
        if(end_of_dispacher)
                return NULL;
        //printf("wake up\n" );
        printf("worker %lu \n",myBuffer[worker_position % buffer_size]);
        pthread_mutex_lock(&mutex_filetw);
        //pthread_cond_signal(&buffer_fill);
        FILE *file = fopen("/home/pouria/Desktop/Info2.txt", "a");
        //;
        time_t now = time(0);
        fprintf(file, "%lu %lu\n",myBuffer[worker_position % buffer_size] ,now);
        fclose(file);
        pthread_mutex_unlock(&mutex_filetw);
        worker_position++;
        pthread_cond_signal(&buffer_empty);
        pthread_mutex_unlock(&buffer_mutex);
        usleep(((rand() % 10) + 10) * 1000);
    }

    while(worker_position <dispatcher_position){
      printf("worker %lu \n",myBuffer[worker_position % buffer_size]);
      pthread_mutex_lock(&mutex_filetw);
      //pthread_cond_signal(&buffer_fill);
      FILE *file = fopen("/home/pouria/Desktop/Info2.txt", "a");
      //;
      time_t now = time(0);
      fprintf(file, "%lu %lu\n",myBuffer[worker_position % buffer_size] ,now);
      fclose(file);
      pthread_mutex_unlock(&mutex_filetw);
      worker_position++;
    }
    printf("worker counter %d\n",worker_position );
    return NULL;
}

void *collector(void *arg){
  unsigned long i;
  unsigned long j;
  unsigned long diffrencetime=0;
  float diffrencetime2=0;
  float diverjance=0;
  static const char filename[] = "/home/pouria/Desktop/Info2.txt";
  FILE *file = fopen(filename, "r");
  int count = 0;
  if ( file != NULL )
  {
    char * pch;
    // printf ("Splitting string \"%s\" into tokens:\n",str);

      char line[512]; /* or other suitable maximum line size */
      while (fgets(line, sizeof line, file) != NULL) /* read a line */
      {
              //use line or in a function return it
              //printf("%s",line);
              //in case of a return first close the file with "fclose(file);"
              //int i;
              //int j;
              //printf("HI\n" );
              pch = strtok (line," \n");
              //printf ("%s\n",pch);
              i=atoi(pch);

               //printf ("%s\n",pch);
                pch = strtok (NULL, " \n");
                //printf("goodbye\n" );
                //printf ("%s\n",pch);
                j=atoi(pch);
                diffrencetime+=j-i;
                //printf("%lu \n",diffrencetime);

      }
      fclose(file);
  }

  float average=((float)diffrencetime)/((float)counting_dispatcher );
  printf("the average is %f\n", average );
  FILE *myfile = fopen(filename, "r");
  //int count = 0;
  if ( file != NULL )
  {
    char * pch;
    // printf ("Splitting string \"%s\" into tokens:\n",str);

      char line[512]; /* or other suitable maximum line size */
      while (fgets(line, sizeof line, file) != NULL) /* read a line */
      {

              pch = strtok (line," \n");
              //printf ("%s\n",pch);
              i=atoi(pch);

                pch = strtok (NULL, " \n");
                j=atoi(pch);
                diffrencetime2=average-((float)j-i);
                diverjance+=(diffrencetime2*diffrencetime2);
                //printf("%lu \n",diffrencetime);


      }
      fclose(file);
  }

  printf("diverjance is %f\n", diverjance/(float)counting_dispatcher );
}


int main(int argc, char *argv[]) {
    //FILE *theFile = fopen("/home/pouria/Desktop/Info.txt", "w");
    unsigned long shared_buffer[buffer_size];
    FILE *file = fopen("/home/pouria/Desktop/Info2.txt", "w");
    fclose(file);
    pthread_t producerThread, dispatcherThread,collectorThread;
    pthread_t workeres[workers];
    int rc;
    printf("main: begin\n");
    rc = pthread_create(&producerThread, NULL, producer, NULL);
    assert(rc == 0);
    struct buf_thread BUF_THREAD;
    //BUF_THREAD.file = theFile;
    BUF_THREAD.buffer = shared_buffer;

    rc = pthread_create(&dispatcherThread, NULL, dispatcher, (void *) &BUF_THREAD);
    assert(rc == 0);
    for (int i = 0; i < workers; i++) {
        BUF_THREAD.numTH=i;
        rc = pthread_create(&workeres[i], NULL, worker,(void *) &BUF_THREAD);
        assert(rc == 0);
    }

    rc = pthread_join(producerThread, NULL);
    assert(rc == 0);
    rc = pthread_join(dispatcherThread, NULL);
    assert(rc == 0);
    for (int i = 0; i < workers; i++) {
        rc = pthread_join(workeres[i], NULL);
        assert(rc == 0);
    }
    rc = pthread_create(&collectorThread, NULL, collector, NULL);
    assert(rc == 0);
    rc = pthread_join(collectorThread, NULL);
    assert(rc == 0);
    printf("main: end\n");
    return 0;
}
