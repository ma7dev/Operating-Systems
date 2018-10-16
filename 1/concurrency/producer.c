#include "producer.h"
#include <stdio.h>
#include <unistd.h>

int produce()
{
    displayBuffer();

    if(!isBufferFull()){
        int id = generateRandomInt(0,99);
        int t = generateRandomInt(2,9);
        addNewTask(id,t);
        printf("\t|PRODUCER| Adding task: [%i,%i]\n", id, t);
        fflush(NULL);
        return generateRandomInt(3,7);
    }
    else{
        int sleepTime = generateRandomInt(3,7);
        printf("\t|PRODUCER| Buffer full, Sleeping for: %i\n", sleepTime);
        fflush(NULL);
        return sleepTime;
    }
    return 0;
}

