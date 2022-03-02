#include "embeddedLang.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

eL_Stack_t* new_eL_Stack() {
    eL_Stack_t* stack = (eL_Stack_t*)malloc(sizeof(eL_Stack_t));
    stack->head = NULL;
    return stack;
}

int eL_Stack_push(eL_Stack_t* stack, eL_Value_t value) {
    if(stack->head == NULL) {
        stack->head = (eL_Node_t*)malloc(sizeof(eL_Node_t));

        if(!stack->head) // Check for malloc failure
            return 1;

        stack->head->prev = NULL;
    } else {
        eL_Node_t* old_head = stack->head;
        stack->head = (eL_Node_t*)malloc(sizeof(eL_Node_t));

        if(!stack->head) // Check for malloc failure
            return 1;

        stack->head->prev = old_head;
    }

    stack->head->value = value;

    return 0;
}

eL_Value_t eL_Stack_Pop(eL_Stack_t* stack) {
    eL_Value_t poped_value = {NONE, NULL};

    if(!stack->head)
        return poped_value;

    poped_value = stack->head->value;

    if(stack->head->prev == NULL) {
        free(stack->head);
        stack->head = NULL;

        return poped_value;
    }

    eL_Node_t* new_head = stack->head->prev;
    free(stack->head);
    stack->head = new_head;

    return poped_value;
}

eL_Value_t eL_Stack_Peek(eL_Stack_t* stack) {
    eL_Value_t peeked_value = {NONE, NULL};

    if(!stack->head)
        return peeked_value;

    peeked_value = stack->head->value;

    return peeked_value;
}
/*-----------------------------------*/

eL_VM_t* new_eL_VM() {
    eL_VM_t* vm = (eL_VM_t*)malloc(sizeof(eL_VM_t));

    vm->data_stack = new_eL_Stack();
    vm->return_stack = new_eL_Stack();

    vm->code_list = NULL;
    vm->function_list = NULL;

    vm->current_operand = 0;

    vm->done = 0;

    return vm;
}

int eL_VM_load(eL_VM_t* vm, char code[]) {

    /*-----------------------------------*/

    while(code[strlen(code) - 1] == ' ' || code[strlen(code) - 1] == '\n')
        code[strlen(code) - 1] = '\0';

    code[strlen(code) - 1] = '\0';

    char** result = 0;
    size_t code_count = 0;
    size_t func_count = 0;
    char* code_tmp = code;
    char* func_tmp = code;
    char* last_delim = NULL;
    char delim[] = ";";

    while(*code_tmp) {
        if(delim[0] == *code_tmp) {
            code_count++;
            last_delim = code_tmp;
        }
        code_tmp++;
    }

    while(*func_tmp) {
        if(':' == *func_tmp) {
            func_count++;
            last_delim = func_tmp;
        }
        func_tmp++;
    }

    code_count += last_delim < (code + strlen(code) - 1);
    code_count++;

    result = (char**)malloc(sizeof(char*) * code_count);

    if(!result)
        return 1;

    /*-----------------------------------*/

    size_t idx = 0;

    char* token = strtok(code, delim);

    while(token) {
        assert(idx < code_count);
        *(result + idx++) = strdup(token);
        token = strtok(0, delim);
    }
    assert(idx == code_count - 1);
    *(result + idx) = 0;

    vm->code_list = malloc(sizeof(eL_Value_t*) * code_count);
    vm->code_lenght = code_count - 1;

    vm->function_list = malloc(sizeof(eL_Function_t*) * func_count);
    vm->func_list_lenght = func_count;
    
    /*-----------------------------------*/

    for(int i = 0; *(result + i); i ++) {
        char* operand = *(result + i);

        while(operand[0] == ' ' || operand[0] == '\n')
            memmove(operand, operand+1, strlen(operand));

        while(operand[strlen(operand) - 1] == ' ' || operand[strlen(operand) - 1] == '\n')
            operand[strlen(operand) - 1] = '\0';


        eL_Value_t *value = malloc(sizeof(eL_Value_t));

        if(isdigit(operand[0])) { // It's a number
            if(strchr(operand, '.') != NULL) { // It's a float

                value->type = FLOAT;
                value->contents = malloc(sizeof(float));
                *(float*)value->contents = atof(operand);

            } else {                           // It's a int

                value->type = INT;
                value->contents = malloc(sizeof(int));
                *(int*)value->contents = atoi(operand);

            }
        } else if(operand[0] == '\'' || operand[0] == '"') { // It's a string
            
            value->type = STRING;
            value->contents = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(value->contents, operand);
            memmove((char*)value->contents, (char*)value->contents+1, strlen(value->contents));
            ((char*)value->contents)[strlen(value->contents) - 1] = '\0';

        } else if(operand[0] == '@') { // It's a function call
            
            value->type = FUNCTION_CALL;
            value->contents = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(value->contents, operand);
            memmove((char*)value->contents, (char*)value->contents+1, strlen(value->contents));

        } else if(operand[0] == '@' && operand[1] == '?') { // It's a conditional function call

            value->type = CONDITIONAL_FUNCTION_CALL;
            value->contents = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(value->contents, operand);
            memmove((char*)value->contents, (char*)value->contents+2, strlen(value->contents));

        } else if(operand[strlen(operand)-1] == ':') { // It's a function label

            value->type = FUNCTION_LABEL;
            value->contents = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(value->contents, operand);

            eL_Function_t *func = malloc(sizeof(eL_Function_t));

            func->index = i;
            func->label = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(func->label, operand);
            func->label[strlen(func->label) - 1] = '\0';

            *(vm->function_list + func_count-1) = func;

            if(strcmp(operand, "main") == 0)
                vm->current_operand = i;

            func_count--;
            // printf("Function %d %s\n", (*(vm->function_list + i))->index, (*(vm->function_list + i))->label);

        } else { // It's language operator
            value->type = LANG_OPERAND;
            value->contents = malloc(sizeof(char) * (strlen(operand) + 1));
            strcpy(value->contents, operand);

        }

        *(vm->code_list + i) = value;

    }

    free(result);
}

int eL_VM_tick(eL_VM_t* vm) {
    if(vm->done) 
        return 1;

    eL_Value_t op = **(vm->code_list+vm->current_operand);

    if(op.type == NONE)
        return 0;

    switch(op.type) {
        case STRING:
        case INT:
        case FLOAT:
            eL_Stack_push(vm->data_stack, op);
            vm->current_operand++;
            break;

        case LANG_OPERAND:
            if(!strcmp("print", op.contents) || !strcmp("println", op.contents)) {
                eL_Value_t val = eL_Stack_Peek(vm->data_stack);

                switch (val.type) {
                    case STRING:
                        printf("%s", val.contents);
                        break;

                    case INT:
                        printf("%d", *(int*)val.contents);
                        break;

                    case FLOAT:
                        printf("%f", *(float*)val.contents);
                        break;
                    
                    default:
                        break;
                }

                if(!strcmp("println", op.contents))
                    printf("\n");
            } else
            if(!strcmp("return", op.contents)) {
                if(eL_Stack_Peek(vm->return_stack).type == NONE)
                    vm->done = 1;
            } else
            if(!strcmp("drop", op.contents)) {
                eL_Stack_Pop(vm->data_stack);
            } else
            if(!strcmp("+", op.contents)) {
                eL_Value_t A = eL_Stack_Pop(vm->data_stack);
                eL_Value_t B = eL_Stack_Pop(vm->data_stack);

                if(A.type == NONE && B.type != NONE) {
                    eL_Stack_push(vm->data_stack, B);
                    vm->current_operand++;
                    break;
                }

                if(B.type == NONE && A.type != NONE) {
                    eL_Stack_push(vm->data_stack, A);
                    vm->current_operand++;
                    break;
                }

                if(A.type == STRING && B.type == STRING) {
                    eL_Stack_push(vm->data_stack, A);
                    eL_Stack_push(vm->data_stack, B);
                    vm->current_operand++;
                    break;
                }

                eL_Value_t C;

                if(A.type == FLOAT || B.type == FLOAT) {
                    C.type = FLOAT;
                    C.contents = malloc(sizeof(float));
                    *(float*)C.contents = *(float*)A.contents + *(float*)B.contents;

                    eL_Stack_push(vm->data_stack, C);
                    vm->current_operand++;
                    break;
                }

                C.type = INT;
                C.contents = malloc(sizeof(int));
                *(int*)C.contents = *(int*)A.contents + *(int*)B.contents;

                eL_Stack_push(vm->data_stack, C);
                vm->current_operand++;
                break;
            }else {}
            vm->current_operand++;
            break;

        default:
            vm->current_operand++;
            break;
        
    }

    return 0;
}
