#ifndef ERRORS_H
#define ERRORS_H

#include "node.h"
#include "scope.h"

void errors_check_function_call(struct Node *def, struct Node *call, struct Scope *scope);

#endif

