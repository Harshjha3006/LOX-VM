#ifndef debug_h
#define debug_h

#include "chunk.h"

void disAssembleChunk(Chunk *chunk,const char *name);
int disAssembleInstruction(Chunk *chunk,int offset);

#endif