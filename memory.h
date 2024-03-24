#ifndef memory_h
#define memory_h

#include "common.h"


// decides new Capacity value once capacity is full
#define GROW_CAPACITY(capacity) ((capacity < 8) ? 8 : 2 * capacity);


// dynamically grows the array 
#define GROW_ARRAY(type,pointer,oldCapacity,newCapacity) (type*)reallocate(pointer,sizeof(type) * oldCapacity,sizeof(type) * newCapacity)


// frees the array
#define FREE_ARRAY(type,pointer,capacity) reallocate(pointer,sizeof(type) * capacity,0)

// resizes pointed memory block from oldSize to newSize
void* reallocate(void *pointer,size_t oldSize,size_t newSize);

#endif