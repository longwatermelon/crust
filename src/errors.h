#ifndef ERRORS_H
#define ERRORS_H

#include "node.h"
#include "scope.h"
#include "asm.h"

void errors_check_function_call(struct Node *def, struct Node *call, struct Asm *as);
void errors_check_function_return(struct Node *def, struct Asm *as);
// Always call this function before inserting the new function definition into scope, or else it won't work properly
void errors_check_function_def(struct Node *def, struct Asm *as);

void errors_check_variable_def(struct Node *def, struct Asm *as);
void errors_check_assignment(struct Node *assignment, struct Asm *as);

void errors_print_lines(struct Asm *as, size_t line, size_t range);
void errors_print_line(struct Asm *as, size_t line);

#endif

