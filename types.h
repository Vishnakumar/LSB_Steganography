#ifndef TYPES_H
#define TYPES_H

/* User defined data types */
typedef unsigned int uint;

/* Status will be used in fn. return type */
typedef enum
{
    failure,
    success
} Status;

typedef enum
{
    //enumerators
    e_encode,       //0
    e_decode,       //1
    e_unsupported   //2
} OperationType;

/* Function prototype */
OperationType check_operation_type(char *);

#endif
