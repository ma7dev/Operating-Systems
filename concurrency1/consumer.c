#include "consumer.h"
#include <unistd.h>
#include <stdio.h>

int consume()
{
    displayBuffer();

    if(!isBufferEmpty()){
        int id = buffer[bufferBottom % BUFFER_SIZE].id;
        int sleepTime = removeNextTask();
        printf("\t|CONSUMER| Removed [%i,%i]. Sleeping for: %i\n", id, sleepTime, sleepTime);
        fflush(NULL);
        return sleepTime;
    }
    else{
        int sleepTime = generateRandomInt(2,9);
        printf("\t|CONSUMER| Buffer empty, Sleeping for: %i\n", sleepTime);
        fflush(NULL);
        return sleepTime;
    }
    return 0;
}


