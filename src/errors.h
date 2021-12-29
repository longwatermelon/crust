#ifndef ERRORS_H
#define ERRORS_H

#include "node.h"
#include "scope.h"

void errors_check_function_call(struct Scope *scope, struct Node *def, struct Node *call);

#endif

