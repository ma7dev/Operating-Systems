#ifndef PRODUCER_H
#define PRODUCER_H

#include "rand.h"
#include "task.h"

/*
Adds an element to the buffer. 
*/
void produce();

/*
Returns 0 for empty, 1 for non-empty
*/
int isBufferFull();

#endif