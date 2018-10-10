#include "consumer.h"
#include "producer.h"

#include <stdio.h>

//Global buffer of tasks
struct task buffer[25];

int main(int argc, char* argv[])
{
    buffer[0].id = 100;
    buffer[0].time = 2;

    printf("id: %i | time: %i \n", buffer[0].id, buffer[0].time);

    /*
    TODO
    Create a thread for the producer, and for the consumer.
    Call consume(), and produce() which should then call their respective sleep functions.
    Run forever?
    */

   return 0;
}