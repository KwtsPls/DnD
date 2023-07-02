#ifndef DND_PARSER_H
#define DND_PARSER_H

#include <glib.h>
#include "tokenizer.h"

typedef enum{SELECT_STMT,INSERT_STMT,UPDATE_STMT}StatementType;

typedef struct statement{
    StatementType type;
    GList *vars;
    GList *tables;
}Statement;

typedef struct var{
    TokenType type;
    char *table;
    char *column;
}Var;

//Parse a list of tokens and convert it to a statement
Statement *parse_statement(GList *tokens);

//Function to parse select statements
Statement *parse_select(GList *tokens,Statement *statement);
//Function to parse a list of result variables
Statement *parse_var_list(GList *tokens,Statement *statement);
//Function to parse a single var
Statement *parse_var(Token *var_token,GList *tokens,Statement *statement);
//Function to parse a single aggregate
Var *parse_aggr(Token *var_token,GList *tokens,Statement *statement);

#endif //DND_PARSER_H
