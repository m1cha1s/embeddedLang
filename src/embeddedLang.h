#ifndef EMBEDDEDLANG_H
#define EMBEDDEDLANG_H

typedef enum eL_Type_t {
    NONE,
    STRING,
    INT,
    FLOAT,

    FUNCTION_LABEL,
    FUNCTION_CALL,
    CONDITIONAL_FUNCTION_CALL,
    LANG_OPERAND,
} eL_Type_t;

/*-----------------------------------*/

typedef struct eL_Value_t {
    eL_Type_t type;
    void* contents;
} eL_Value_t;

typedef struct eL_Function_t {
    char *label;
    unsigned int index;
} eL_Function_t;

/*-----------------------------------*/

typedef struct eL_Node_t {
    struct eL_Node_t* prev;
    eL_Value_t value;
}eL_Node_t;

typedef struct eL_Stack_t {
    eL_Node_t* head;
} eL_Stack_t;

eL_Stack_t* new_eL_Stack();
int eL_Stack_push(eL_Stack_t* stack, eL_Value_t value);
eL_Value_t eL_Stack_Pop(eL_Stack_t* stack);
eL_Value_t eL_Stack_Peek(eL_Stack_t* stack);

/*-----------------------------------*/

typedef struct eL_VM_t {
    eL_Stack_t *data_stack, *return_stack;

    eL_Value_t **code_list;
    eL_Function_t **function_list;

    unsigned int current_operand, code_lenght, func_list_lenght, done;
}eL_VM_t;

eL_VM_t* new_eL_VM();
int eL_VM_load(eL_VM_t* vm, char* code);
int eL_VM_tick(eL_VM_t* vm);

#endif