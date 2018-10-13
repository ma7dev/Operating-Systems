#include "consumer.h"
#include "producer.h"
#include "task.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    //Little testing code
    produce();
    produce();
    produce();
    consume();
    int j;
    for(j=0; j<BUFFER_SIZE; j++){
        printf("[%i,%i],", buffer[j].id, buffer[j].time);
    }
    printf("\n");

    /*
    TODO
    Create a thread for the producer, and for the consumer.
    Call consume(), and produce() which should then call their respective sleep functions.
    Run forever?
    */

   return 0;
}