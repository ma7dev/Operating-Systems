#include "task.h"

//Global buffer of tasks
struct task buffer[BUFFER_SIZE];

//Index of top of buffer
int bufferTop = 0;

//Index of bottom of buffer
int bufferBottom = 0;

void addNewTask(int id, int time){
    struct task newTask;
    newTask.id = id;
    newTask.time = time; 

    buffer[bufferTop % BUFFER_SIZE] = newTask;
    bufferTop++;
}

void removeNextTask(){
    struct task newTask;
    newTask.id = 0;
    newTask.time = 0; 
    buffer[bufferBottom % BUFFER_SIZE] = newTask;
    bufferBottom++;
}