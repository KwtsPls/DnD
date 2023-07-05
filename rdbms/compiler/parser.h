#ifndef DND_PARSER_H
#define DND_PARSER_H

#include <glib.h>
#include "tokenizer.h"

typedef enum{SELECT_STMT,INSERT_STMT,UPDATE_STMT}StatementType;

typedef struct var{
    TokenType type;
    char *table;
    char *column;
}Var;

typedef struct table{
    char *name;
}Table;


typedef struct expr{
    TokenType x_type;
    void *x;
    TokenType y_type;
    void *y;
    TokenType op;
}Expr;

typedef struct boolean_expr{
    Expr **expr;
    int size;
}BooleanExpr;

typedef struct order{
    Var *var;
    int isAsc;
}Order;

typedef struct statement{
    StatementType type;
    GList *vars;
    GList *tables;
    BooleanExpr *expr;
    Var *group;
    Order *order;
    int limit;
}Statement;

//Parse a list of tokens and convert it to a statement
Statement *parse_statement(GList **tokens);

//Function to parse select statements
Statement *parse_select(GList **tokens,Statement *statement);
//Function to parse a list of result variables
Statement *parse_var_list(GList **tokens,Statement *statement);
//Function to parse a list of tables (FROM clause)
Statement *parse_table_list(GList **tokens,Statement *statement);
//Function to parse the where clause of a query
Statement *parse_where_clause(GList **tokens,Statement *statement);
//Function to parse the group by clause of a query
Statement *parse_group_clause(GList **tokens,Statement *statement);
//Function to parse the order by clause of a query
Statement *parse_order_clause(GList **tokens,Statement *statement);
//Function to parse the order by clause of a query
Statement *parse_limit_clause(GList **tokens,Statement *statement);

//Function to parse a single var
Var *parse_var(Token *var_token,GList **tokens);
//Function to parse a single aggregate
Var *parse_aggr(Token *var_token,GList **tokens);
//Function to parse a single table
Table *parse_table(Token *table_token,GList **tokens);
//Function to parse an expr of the form a op b
Expr *parse_expr(GList **tokens);

//Function to create a new table
Table *table_create(char *name);

//Function to free a statement
void statement_free(Statement *statement);
//Function to free a var
void var_free(void *var);
//Function to free a table
void table_free(void *table);
//Function to free a boolean expr
void boolean_expr_free(BooleanExpr *expr);
//Function to free an expr
void expr_free(void *_expr);

//Function to print an expr
void expr_print(Expr *expr);
//Function to print a statement
void statement_print(Statement *statement);
//Function to print a var
void var_print(void *_var,void *extra);
//Function to print a table
void table_print(void *_table,void *extra);

//Function to check if a token is a comparison operator
int isComparisonOperator(Token *token);
//Create expression operand
void *parse_expr_operand(Token *token,TokenType *type);
//Helper function to pop an element from a glib list
void *g_list_pop(GList **l);

#endif //DND_PARSER_H
