#include "task.h"

//Global buffer of tasks
struct task buffer[BUFFER_SIZE];

//Index of top of buffer
int bufferTop = 0;

//Index of bottom of buffer
int bufferBottom = 0;

void addNewTask(int id, int time)
{
    struct task newTask;
    newTask.id = id;
    newTask.time = time; 

    buffer[bufferTop % BUFFER_SIZE] = newTask;
    bufferTop++;
}

int removeNextTask()
{
    int time = buffer[bufferBottom % BUFFER_SIZE].time;
    struct task newTask;
    newTask.id = 0;
    newTask.time = 0; 
    buffer[bufferBottom % BUFFER_SIZE] = newTask;
    bufferBottom++;
    return time;
}

void displayBuffer()
{
    int j;
    for(j=bufferBottom; j<(bufferBottom+BUFFER_SIZE); j++){
        if((j+1-bufferBottom)%8 != 0){
            printf("[%i,%i]\t", buffer[j%BUFFER_SIZE].id, buffer[j%BUFFER_SIZE].time);
        }
        else{
            printf("[%i,%i]\n", buffer[j%BUFFER_SIZE].id, buffer[j%BUFFER_SIZE].time);
        }
    }
}

/*
Returns 1 for full, 0 for non-full
*/
int isBufferFull()
{
    if(((bufferTop%BUFFER_SIZE) == (bufferBottom%BUFFER_SIZE)) && (bufferTop != 0 || bufferBottom != 0) && (bufferTop != bufferBottom)){
        //Full
        return 1;
    }
    
    else{
        //Not full
        return 0;
    }
    // Unexpected
    return -1;
}

/*
Returns 1 for empty, 0 for non-empty
*/
int isBufferEmpty()
{
    if(((bufferTop%BUFFER_SIZE) == (bufferBottom%BUFFER_SIZE)) && (buffer[bufferBottom].time == 0) && (bufferTop == bufferBottom)){
        //Full
        return 1;
    }
    
    else{
        //Not full
        return 0;
    }
    // Unexpected
    return -1;
}