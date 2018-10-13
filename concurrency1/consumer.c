#include "consumer.h"

/*
Takes the next element off the buffer
*/
void consume()
{
    int sleepTime;
    if(!isBufferEmpty()){
        sleepTime = removeNextTask();
    }
    //Add sleep
}


