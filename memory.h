#ifndef memory_h
#define memory_h

#include "common.h"
#include "object.h"


#define GC_GROW_RATE 2

// decides new Capacity value once capacity is full
#define GROW_CAPACITY(capacity) ((capacity < 8) ? 8 : 2 * capacity);


// dynamically grows the array 
#define GROW_ARRAY(type,pointer,oldCapacity,newCapacity) (type*)reallocate(pointer,sizeof(type) * oldCapacity,sizeof(type) * newCapacity)


// frees the array
#define FREE_ARRAY(type,pointer,capacity) reallocate(pointer,sizeof(type) * capacity,0)

#define FREE(type,pointer) reallocate(pointer,sizeof(type),0)

// resizes pointed memory block from oldSize to newSize
void* reallocate(void *pointer,size_t oldSize,size_t newSize);

// starts the garbage collector

void collectGarbage();

// marks an object
void markObject(Obj*object);

#define ALLOCATE(type,size) (type*)reallocate(NULL,0,sizeof(type) * size)

#endif