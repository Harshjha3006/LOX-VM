#ifndef value_h
#define value_h


typedef double Value;

// struct for dynamic array of constants in the program
typedef struct{
    Value*values;
    int capacity;
    int size;
}ValueArray;

// initialises the value array
void initValueArray(ValueArray*array);
// appending a value in the array
void writeValueArray(ValueArray *array,Value value);
// freeing the value array
void freeValueArray(ValueArray *array);
// printing the value
void printValue(Value value);


#endif