#ifndef ERRORS_H
#define ERRORS_H

#include "node.h"
#include "scope.h"

void errors_check_function_call(struct Node *def, struct Node *call, struct Scope *scope);
void errors_check_function_return(struct Node *def, struct Scope *scope);

void errors_check_variable_def(struct Node *def);
void errors_check_assignment(struct Node *assignment, struct Scope *scope);

#endif

