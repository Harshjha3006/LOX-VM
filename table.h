#ifndef table_h
#define table_h
#define TABLE_MAX_LOAD 0.75

#include "common.h"
#include "value.h"

typedef struct{
    ObjString* key;
    Value value;
}Entry;


typedef struct{
    int count;
    int capacity;
    Entry *entries;
}Table;


void initTable(Table *table);
void freeTable(Table *table);
bool tableSet(Table*table,ObjString*key,Value value);
bool tableGet(Table*table,ObjString*key,Value *value);
bool tableDelete(Table*table,ObjString*key);

#endif