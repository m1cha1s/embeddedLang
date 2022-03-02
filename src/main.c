#include <stdio.h>
#include <string.h>

#include "embeddedLang.h"

/*

Stack based language

program {
    Data stack for data
    Return stack for return positions
    ---------------------------------
    Entry point on first line
}

basic stack example:

2  // Push 2 onto data stack
3  // Push 3 onto data stack
+  // Pop 2 & 3 from data stack add them and push result(5) onto data stack
print  // Peek data stack and print it

compact stack example:

2 3 + print  // Same as above but in one line

function example:

main:
:hello // Call hello & push next address to return stack
print // Peek data stack and print it
return

hello: // label inticator for function "hello"
"hello" // Push "hello" onto data stack
return // Pop adress from 

*/

int main() {
    char code[] = "main:; 2; 3; +; println; 7; +; println; return;  ";
    eL_VM_t* vm = new_eL_VM();
    eL_VM_load(vm, code);

    // printf("%d\n\n", vm->current_operand);

    int err = 0;
    while(!err)
        err = eL_VM_tick(vm);

    printf("Exited with code: %d\n", err - 1);

    return 0;
}