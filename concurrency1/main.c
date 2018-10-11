#include "consumer.h"
#include "producer.h"
#include "task.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    buffer[0].id = 100;
    buffer[0].time = 2;

    int i;
    for(i=0; i<BUFFER_SIZE; i++){
        printf("id: %i | time: %i \n", buffer[i].id, buffer[i].time);
    }

    /*
    TODO
    Create a thread for the producer, and for the consumer.
    Call consume(), and produce() which should then call their respective sleep functions.
    Run forever?
    */

   return 0;
}