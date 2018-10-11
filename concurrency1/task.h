#ifndef TASK_H
#define TASK_H

#define BUFFER_SIZE 25

struct task
{
    //Random id 0-255
    int id;
    //Time to finish task 2-9 seconds
    int time;
};

void addNewTask(int id, int time);
void removeNextTask();

extern struct task buffer[];

#endif