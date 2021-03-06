#include "parser.h"
#include "util.h"
#include "errors.h"
#include "crust.h"

#include <stdio.h>
#include <string.h>


struct Parser *parser_alloc(struct Token **tokens, size_t ntokens, struct Args *args)
{
    struct Parser *parser = malloc(sizeof(struct Parser));
    parser->tokens = tokens;
    parser->ntokens = ntokens;

    parser->curr_idx = 0;
    parser->curr_tok = parser->tokens[parser->curr_idx];

    parser->scope = scope_alloc();
    scope_push_layer(parser->scope);

    parser->stack_size = 4;
    parser->lc = 0;

    parser->args = args;

    parser->prev_node = 0;

    return parser;
}


void parser_free(struct Parser *parser)
{
    scope_free(parser->scope);
    free(parser);
}


void parser_eat(struct Parser *parser, int type)
{
    if (parser->curr_tok->type != type)
        errors_parser_unexpected_token(type, parser->curr_tok);
    else
        parser_advance(parser, 1);
}


void parser_advance(struct Parser *parser, int i)
{
    if (parser->curr_idx + i < parser->ntokens &&
        (int)parser->curr_idx + i >= 0)
    {
        parser->curr_idx += i;
        parser->curr_tok = parser->tokens[parser->curr_idx];
    }
}


struct Node *parser_parse_compound(struct Parser *parser)
{
    struct Node *root = node_alloc(NODE_COMPOUND);

    while (parser->curr_idx < parser->ntokens)
    {
        struct Node *expr = parser_parse_expr(parser, false);

        if (!expr)
            break;

        root->compound_nodes = realloc(root->compound_nodes,
                                   sizeof(struct Node*) * ++root->compound_size);
        root->compound_nodes[root->compound_size - 1] = expr;

        parser_eat(parser, TOKEN_SEMI);
    }

    return root;
}


struct Node *parser_parse_expr(struct Parser *parser, bool ignore_ops)
{
    struct Node *node;

    switch (parser->curr_tok->type)
    {
    case TOKEN_INT: node = parser_parse_int(parser); break;
    case TOKEN_STRING: node = parser_parse_str(parser); break;
    case TOKEN_ID: node = parser_parse_id(parser); break;
    case TOKEN_BINOP: node = parser_parse_binop(parser); break;
    default: node = 0; break;
    }

    parser->prev_node = node;

    if (node && !ignore_ops)
    {
        if (parser->curr_tok->type == TOKEN_BINOP)
            return parser_parse_binop(parser);

        if (parser->curr_tok->type == TOKEN_EQUALS)
            return parser_parse_assignment(parser);
    }

    return node;
}


struct Node *parser_parse_int(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INT);
    node->int_value = atoi(parser->curr_tok->value);
    node->error_line = parser->curr_tok->line_num;

    parser_eat(parser, TOKEN_INT);
    return node;
}


struct Node *parser_parse_str(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_STRING);
    node->string_value = util_strcpy(parser->curr_tok->value);
    node->error_line = parser->curr_tok->line_num;
    node->string_asm_id = parser_next_lc(parser);

    parser_eat(parser, TOKEN_STRING);
    return node;
}


struct Node *parser_parse_id(struct Parser *parser)
{
    if (strcmp(parser->curr_tok->value, "fn") == 0)
        return parser_parse_function_def(parser);
    else if (strcmp(parser->curr_tok->value, "return") == 0)
        return parser_parse_return(parser);
    else if (strcmp(parser->curr_tok->value, "let") == 0)
        return parser_parse_variable_def(parser);
    else if (strcmp(parser->curr_tok->value, "struct") == 0)
        return parser_parse_struct(parser);
    else if (strcmp(parser->curr_tok->value, "include") == 0)
        return parser_parse_include(parser);
    else if (strcmp(parser->curr_tok->value, "idof") == 0)
        return parser_parse_idof(parser);
    else if (strcmp(parser->curr_tok->value, "asm") == 0)
        return parser_parse_inline_asm(parser);
    else if (strcmp(parser->curr_tok->value, "if") == 0)
        return parser_parse_if_statement(parser);
    else
        return parser_parse_variable(parser);
}


struct Node *parser_parse_function_def(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_FUNCTION_DEF);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID); // fn

    node->function_def_name = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_ID); // function name

    size_t prev_size = parser->stack_size;
    parser->stack_size = 4;
    scope_push_layer(parser->scope);

    node->function_def_params = parser_parse_function_def_params(parser, &node->function_def_params_size);

    parser_eat(parser, TOKEN_ARROW);

    node->function_def_return_type = parser_parse_dtype(parser);

    scope_add_function_def(parser->scope, node);

    if (parser->curr_tok->type == TOKEN_SEMI)
    {
        node->function_def_is_decl = true;
    }
    else
    {
        node->function_def_is_decl = false;
        parser_eat(parser, TOKEN_LBRACE);

        if (parser->curr_tok->type != TOKEN_RBRACE)
            node->function_def_body = parser_parse_compound(parser);
        else
        {
            node->function_def_body = node_alloc(NODE_COMPOUND);
            node->function_def_body->compound_nodes = 0;
            node->function_def_body->compound_size = 0;
        }

        parser_eat(parser, TOKEN_RBRACE);
    }

    scope_pop_layer(parser->scope);
    parser->stack_size = prev_size;

    return node;
}


struct Node **parser_parse_function_def_params(struct Parser *parser, size_t *nparams)
{
    struct Node **params = 0;
    *nparams = 0;

    parser_eat(parser, TOKEN_LPAREN);

    size_t offset = 8;

    while (parser->curr_tok->type != TOKEN_RPAREN)
    {
        struct Node *param = node_alloc(NODE_VARIABLE);
        param->error_line = parser->curr_tok->line_num;
        param->variable_stack_offset = offset;
        offset += 4;

        param->variable_name = util_strcpy(parser->curr_tok->value);
        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);
        param->variable_type = parser_parse_dtype(parser);

        param->variable_is_param = true;

        params = realloc(params, sizeof(struct Node*) * ++*nparams);
        params[*nparams - 1] = param;

        if (parser->curr_tok->type == TOKEN_RPAREN)
            break;

        parser_eat(parser, TOKEN_COMMA);
    }

    parser->scope->curr_layer->params = params;
    parser->scope->curr_layer->nparams = *nparams;

    parser_eat(parser, TOKEN_RPAREN);

    return params;
}


struct Node *parser_parse_return(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_RETURN);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);

    node->return_value = parser_parse_expr(parser, false);
    return node;
}


struct Node *parser_parse_variable_def(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_VARIABLE_DEF);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);

    char *name = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_ID);

    parser_eat(parser, TOKEN_COLON);
    node->variable_def_type = parser_parse_dtype(parser);

    if (node->variable_def_type.struct_type)
        scope_find_struct(parser->scope, node->variable_def_type.struct_type, node->error_line);

    parser_eat(parser, TOKEN_EQUALS);

    node->variable_def_name = name;
    node->variable_def_value = parser_parse_expr(parser, false);

    node->variable_def_stack_offset = -parser->stack_size;
    parser->stack_size += node_sizeof_dtype(node_strip_to_literal(node->variable_def_value, parser->scope));

    scope_add_variable_def(parser->scope, node);

    return node;
}


struct Node *parser_parse_variable(struct Parser *parser)
{
    char *variable_name = parser->curr_tok->value;

    if (scope_find_struct(parser->scope, variable_name, -1) &&
        parser->tokens[parser->curr_idx + 1]->type == TOKEN_LBRACE)
        return parser_parse_init_list(parser);

    parser_eat(parser, TOKEN_ID);

    if (parser->curr_tok->type == TOKEN_LPAREN)
        return parser_parse_function_call(parser);

    struct Node *node = node_alloc(NODE_VARIABLE);
    node->error_line = parser->curr_tok->line_num;
    node->variable_name = util_strcpy(variable_name);

    if (parser->curr_tok->type == TOKEN_PERIOD)
    {
        struct Node *def = scope_find_variable(parser->scope, node, node->error_line);
        node->variable_type = node_dtype_copy(node_type_from_node(def, parser->scope));

        if (node->variable_type.type != NODE_STRUCT)
        {
            parser_eat(parser, TOKEN_PERIOD);
            errors_parser_invalid_member_access(parser->scope, node, parser->curr_tok->value);
        }

        node->variable_stack_offset = node_stack_offset(def);
        node->variable_is_param = def->variable_is_param;

        struct Node *member = parser_parse_variable_struct_member(parser, scope_find_struct(
            parser->scope, node->variable_type.struct_type, -1
        ), node->variable_stack_offset);

        node->variable_struct_member = member;

        return node;
    }

    node->variable_type = node_dtype_copy(node_type_from_node(scope_find_variable(parser->scope, node, node->error_line), parser->scope));
    node->variable_stack_offset = scope_find_variable(parser->scope, node, node->error_line)->variable_def_stack_offset;

    return node;
}


struct Node *parser_parse_variable_struct_member(struct Parser *parser, struct Node *parent_struct, int stack_offset)
{
    parser_eat(parser, TOKEN_PERIOD);
    struct Node *node = node_alloc(NODE_VARIABLE);
    node->variable_name = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_ID);

    for (size_t i = 0; i < parent_struct->struct_members_size; ++i)
    {
        if (strcmp(parent_struct->struct_members[i]->member_name, node->variable_name) == 0)
        {
            node->variable_type = node_dtype_copy(parent_struct->struct_members[i]->member_type);
            node->variable_stack_offset = stack_offset - i * 4;
            break;
        }
    }

    if (parser->curr_tok->type == TOKEN_PERIOD)
    {
        node->variable_struct_member = parser_parse_variable_struct_member(parser,
                scope_find_struct(parser->scope, node->variable_type.struct_type, -1),
                node->variable_stack_offset);
    }

    return node;
}


struct Node *parser_parse_function_call(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_FUNCTION_CALL);
    node->error_line = parser->curr_tok->line_num;
    node->function_call_name = util_strcpy(parser->tokens[parser->curr_idx - 1]->value);

    parser_eat(parser, TOKEN_LPAREN);

    while (parser->curr_tok->type != TOKEN_RPAREN)
    {
        struct Node *expr = parser_parse_expr(parser, false);

        node->function_call_args = realloc(node->function_call_args,
            sizeof(struct Node*) * ++node->function_call_args_size);

        node->function_call_args[node->function_call_args_size - 1] = expr;

        if (parser->curr_tok->type != TOKEN_RPAREN)
            parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RPAREN);

    node->function_call_return_stack_offset = -parser->stack_size;

    struct Node *func_def = scope_find_function(parser->scope, node->function_call_name, -1);
    size_t size = 4;

    if (func_def && func_def->function_def_return_type.struct_type)
    {
        size = node_sizeof_dtype(
            scope_find_struct(parser->scope, func_def->function_def_return_type.struct_type, func_def->error_line)
        );
    }

    parser->stack_size += size;

    return node;
}


struct Node *parser_parse_assignment(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_ASSIGNMENT);
    node->error_line = parser->curr_tok->line_num;

    node->assignment_dst = parser->prev_node;
    parser_eat(parser, TOKEN_EQUALS);

    node->assignment_src = parser_parse_expr(parser, false);

    return node;
}


struct Node *parser_parse_struct(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_STRUCT);
    parser_eat(parser, TOKEN_ID);

    node->error_line = parser->curr_tok->line_num;

    node->struct_name = util_strcpy(parser->curr_tok->value);

    scope_add_struct_def(parser->scope, node);

    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LBRACE);

    while (parser->curr_tok->type != TOKEN_RBRACE)
    {
        struct Node *member = node_alloc(NODE_STRUCT_MEMBER);
        member->member_name = util_strcpy(parser->curr_tok->value);

        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);

        member->member_type = parser_parse_dtype(parser);

        node->struct_members = realloc(node->struct_members,
                        sizeof(struct Node*) * ++node->struct_members_size);
        node->struct_members[node->struct_members_size - 1] = member;

        if (parser->curr_tok->type == TOKEN_COMMA)
            parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RBRACE);
    return node;
}


struct Node *parser_parse_init_list(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INIT_LIST);
    node->init_list_type = parser_parse_dtype(parser);
    node->init_list_stack_offset = -parser->stack_size;
    node->error_line = parser->curr_tok->line_num;

    parser_eat(parser, TOKEN_LBRACE);

    while (parser->curr_tok->type != TOKEN_RBRACE)
    {
        struct Node *expr = parser_parse_expr(parser, false);

        node->init_list_values = realloc(node->init_list_values,
                        sizeof(struct Node*) * ++node->init_list_len);
        node->init_list_values[node->init_list_len - 1] = expr;

        if (parser->curr_tok->type == TOKEN_COMMA)
            parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RBRACE);

    return node;
}


struct Node *parser_parse_include(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INCLUDE);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);
    node->include_path = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_STRING);

    char *full_path = util_find_file(parser->args->include_dirs,
            parser->args->include_dirs_len, node->include_path);

    if (!full_path)
        errors_parser_nonexistent_include(node);

    free(node->include_path);
    node->include_path = full_path;

    size_t ntokens;
    struct Token **tokens = crust_tokenize(node->include_path, &ntokens);
    struct Parser *p = parser_alloc(tokens, ntokens, parser->args);
    node->include_root = parser_parse_compound(p);
    scope_combine(parser->scope, p->scope);

    if (!node->include_scope)
    {
        node->include_scope = scope_alloc();
        scope_push_layer(node->include_scope);
    }

    scope_combine(node->include_scope, p->scope);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);
    parser_free(p);

    return node;
}


struct Node *parser_parse_binop(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_BINOP);
    node->error_line = parser->curr_tok->line_num;
    node->op_stack_offset = -parser->stack_size;
    parser->stack_size += 8;

    node->op_type = parser->curr_tok->binop_type;
    parser_advance(parser, 1);

    node->op_l = parser->prev_node;
    node->op_r = parser_parse_expr(parser, true);

    if (parser->curr_tok->type == TOKEN_BINOP)
    {
        parser->prev_node = node;
        struct Node *root = parser_parse_binop(parser);

        return root;
    }
    else
    {
        return node;
    }
}


struct Node *parser_parse_idof(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_IDOF);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);

    struct Node *expr = parser_parse_expr(parser, false);
    node->idof_original_expr = expr;

    struct Node *literal = node_strip_to_literal(expr, parser->scope);
    NodeDType type = node_type_from_node(literal, parser->scope);

    if (type.type != NODE_STRING)
        errors_parser_idof_wrong_type(literal);

    if (literal->type == NODE_STRING)
    {
        struct Node *string = node_alloc(NODE_STRING);
        string->string_value = util_strcpy(literal->string_asm_id);
        string->string_asm_id = parser_next_lc(parser);
        string->error_line = parser->curr_tok->line_num;

        node->idof_new_expr = string;
    }
    else
    {
        node->idof_new_expr = node_copy(literal);
    }

    if (literal->type != NODE_STRING && parser->args->warnings[WARNING_REDUNDANT_IDOF])
        errors_warn_redundant_idof(node);

    return node;
}


struct Node *parser_parse_inline_asm(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INLINE_ASM);
    node->error_line = parser->curr_tok->line_num;

    parser_eat(parser, TOKEN_ID);

    while (parser->curr_tok->type != TOKEN_SEMI)
    {
        node->asm_args = realloc(node->asm_args,
            sizeof(struct Node*) * ++node->asm_nargs);
        node->asm_args[node->asm_nargs - 1] = parser_parse_expr(parser, false);

        if (parser->curr_tok->type != TOKEN_SEMI)
            parser_eat(parser, TOKEN_COMMA);
    }

    return node;
}


struct Node *parser_parse_if_statement(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_IF);
    parser_eat(parser, TOKEN_ID);

    node->if_cond = parser_parse_expr(parser, false);

    parser_eat(parser, TOKEN_LBRACE);
    node->if_body = parser_parse_compound(parser);
    parser_eat(parser, TOKEN_RBRACE);

    return node;
}


NodeDType parser_parse_dtype(struct Parser *parser)
{
    char *name = util_strcpy(parser->curr_tok->value);
    NodeDType type = node_type_from_str(name);

    if (type.type != NODE_STRUCT)
        free(name);

    parser_eat(parser, TOKEN_ID);

    return type;
}


char *parser_next_lc(struct Parser *parser)
{
    char *label = util_int_to_str(parser->lc);
    char *s = calloc(strlen(label) + strlen("$.LC") + 1, sizeof(char));
    sprintf(s, "$.LC%s", label);

    ++parser->lc;
    free(label);

    return s;
}

