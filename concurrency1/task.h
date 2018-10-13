#ifndef TASK_H
#define TASK_H

#define BUFFER_SIZE 5

struct task
{
    //Random id 0-255
    int id;
    //Time to finish task 2-9 seconds
    int time;
};

/*
Adds a task to the next available spot
Sets the specified id and time
*/
void addNewTask(int id, int time);
/*
Removes the oldest task. Returns the time value
*/
int removeNextTask();

/*
Returns 0 for empty, 1 for non-empty
*/
int isBufferFull();
/*
Returns 1 for empty, 0 for non-empty
*/
int isBufferEmpty();

extern struct task buffer[];
//Index of top of buffer
int bufferTop;
//Index of bottom of buffer
int bufferBottom;

#endif