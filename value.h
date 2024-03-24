#ifndef value_h
#define value_h

typedef double Value;

typedef struct{
    Value*values;
    int capacity;
    int size;
}ValueArray;

void initValueArray(ValueArray*array);
void writeValueArray(ValueArray *array,Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);


#endif