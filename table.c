#include "table.h"
#include "object.h"
#include <stdio.h>
#include <inttypes.h>


void initTable(Table *table){
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table){
    FREE_ARRAY(Table,table->entries,table->capacity);
    initTable(table);
}

Entry*findEntry(ObjString*key,int capacity,Entry*entries){
    uint32_t bucket = key->hash & (capacity - 1);
    Entry *tombstone = NULL;
    for(;;){
        Entry*entry = &entries[bucket];
        if(entry->key == NULL){
           if(IS_NIL(entry->value)){
              return tombstone != NULL?tombstone:entry;
           }
           else{
               if(tombstone == NULL)tombstone = entry;
           }
        }
        else if(entry->key == key){
            return entry;
        }
        bucket = (bucket + 1) & (capacity - 1);
    }
}

void growCapacity(Table*table,int capacity){
    Entry*entries = ALLOCATE(Entry,capacity);
    for(int i = 0;i < capacity;i++){
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }
    
    table->count = 0;

    for(int i = 0;i < table->capacity;i++){
        Entry*entry = &table->entries[i];
        if(entry->key == NULL)continue;
        Entry*dest = findEntry(entry->key,capacity,entries);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    FREE_ARRAY(Entry,table->entries,capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table*table,ObjString*key,Value value){

    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD){
        int capacity = GROW_CAPACITY(table->capacity);
        growCapacity(table,capacity);
    }

    Entry*entry = findEntry(key,table->capacity,table->entries);
    bool isNewKey = (entry->key == NULL);
    if(isNewKey && IS_NIL(entry->value))table->count++;
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table*table,ObjString*key,Value *value){
    if(table->count == 0)return false;
    Entry*entry = findEntry(key,table->capacity,table->entries);
    if(entry->key == NULL)return false;
    *value = entry->value;
    return true;
}


bool tableDelete(Table*table,ObjString*key){
    if(table->count == 0)return false;
    Entry*entry = findEntry(key,table->capacity,table->entries);
    if(entry->key == NULL)return false;

    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableCopy(Table*from,Table*to){
    for(int i = 0;i < from->capacity;i++){
        Entry*entry = &from->entries[i];
        if(entry->key == NULL)continue;
        tableSet(to,entry->key,entry->value);
    }
}