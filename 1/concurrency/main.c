#include "consumer.h"
#include "producer.h"
#include "task.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex_lock;

void *consumerThread()
{
    int sleepTime;
    while(1){
        pthread_mutex_lock(&mutex_lock);
        sleepTime = consume();
        pthread_mutex_unlock(&mutex_lock);
        sleep(sleepTime);
    }
}

void *producerThread()
{
    int sleepTime;
    while(1){
        pthread_mutex_lock(&mutex_lock);
        sleepTime = produce();
        pthread_mutex_unlock(&mutex_lock);
        sleep(sleepTime);
    }
}

int main(int argc, char* argv[])
{
    init_genrand(time(NULL));
    
    pthread_mutex_init(&mutex_lock, NULL);
    pthread_mutex_unlock(&mutex_lock);

    pthread_t thread_id_P;
    pthread_t thread_id_C; 

    pthread_create(&thread_id_C, NULL, consumerThread, (void *)&thread_id_C); 
    pthread_create(&thread_id_P, NULL, producerThread, (void *)&thread_id_P);
    
    pthread_join(thread_id_C, NULL);
    pthread_join(thread_id_P, NULL);
    
    pthread_mutex_destroy(&mutex_lock);

   return 0;
}

