#ifndef CONSUMER_H
#define CONSUMER_H

#include "rand.h"
#include "task.h"

/*
Takes the next element off the buffer
*/
void consume();

/*
Returns 0 for empty, 1 for non-empty
*/
int isBufferEmpty();

#endif