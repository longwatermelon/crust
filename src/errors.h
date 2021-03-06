#ifndef ERRORS_H
#define ERRORS_H

#include "node.h"
#include "scope.h"
#include "asm.h"
#include "token.h"
#include "parser.h"

void errors_load_source(char **source, size_t nlines);

/* Errors */
void errors_lexer_unrecognized_char(char c, size_t line);

void errors_parser_unexpected_token(int expected, struct Token *found);
void errors_parser_idof_wrong_type(struct Node *idof_expr);

void errors_parser_nonexistent_include(struct Node *node);

void errors_parser_invalid_member_access(struct Scope *scope, struct Node *node, char *member);

void errors_asm_check_function_call(struct Scope *scope, struct Node *def, struct Node *call);
void errors_asm_check_function_return(struct Scope *scope, struct Node *def);
// Always call this function before inserting the new function definition into scope, or else it won't work properly
void errors_asm_check_function_def(struct Scope *scope, struct Node *def);

void errors_asm_check_variable_def(struct Scope *scope, struct Node *def);
void errors_asm_check_assignment(struct Scope *scope, struct Node *assignment);

void errors_asm_check_init_list(struct Scope *scope, struct Node *list);

void errors_asm_str_from_node(struct Node *node);

void errors_args_nonexistent_warning(char *warning);
void errors_args_no_opt_value(char *opt);

void errors_scope_nonexistent_variable(char *name, size_t line);
void errors_scope_nonexistent_function(char *name, size_t line);
void errors_scope_nonexistent_struct(char *name, size_t line);

/* Warnings */
void errors_warn_dead_code(struct Node *func_def);
void errors_warn_unused_variable(struct Scope *scope, struct Node *func_def);
void errors_warn_print_unused_variable(size_t line, char *var_name);
void errors_warn_redundant_idof(struct Node *idof);

void errors_print_lines(size_t line);
void errors_print_line(size_t line);

#endif

