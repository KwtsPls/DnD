#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "tokenizer.h"

void *g_list_pop(GList **l){
    GList *head = *l;
    *l = g_list_remove_link(*l,head);
    void *data = head->data;
    g_list_free_1(head);
    return data;
}

//Parse a list of tokens and convert it to a statement
Statement *parse_statement(GList **tokens){
    Statement *statement = malloc(sizeof(Statement));
    statement->tables=NULL;
    statement->vars=NULL;
    statement->expr=NULL;
    statement->group=NULL;
    statement->order=NULL;
    statement->limit=0;

    //pop first element of the list
    Token *token = (Token*)g_list_pop(tokens);
    if(token->type==SELECT){
        statement->type = SELECT_STMT;
        statement = parse_select(tokens,statement);
    }
    else{
        printf("Parse error. Expected SELECT not ");
        token_print(token,NULL);
        printf("\n");
        free(statement);
        return NULL;
    }

    if(statement==NULL)
        printf("PARSE ERROR\n");

    token_free(token);
    return statement;
}

//Function to parse select statements
Statement *parse_select(GList **tokens,Statement *statement){
    statement = parse_var_list(tokens,statement);
    if(statement==NULL) return NULL;
    statement = parse_table_list(tokens,statement);
    if(statement==NULL) return NULL;

    //optional clauses in the query
    statement = parse_where_clause(tokens,statement);
    if(statement==NULL) return NULL;
    statement = parse_group_clause(tokens,statement);
    if(statement==NULL) return NULL;
    statement = parse_order_clause(tokens,statement);
    if(statement==NULL) return NULL;
    statement = parse_limit_clause(tokens,statement);
    if(statement==NULL) return NULL;

    return statement;
}

//Function to parse the order by clause of a query
Statement *parse_limit_clause(GList **tokens,Statement *statement){
    if(g_list_length(*tokens)==0){
        statement->limit = 0;
        return statement;
    }

    if(((Token*)g_list_first(*tokens)->data)->type!=LIMIT){
        statement_free(statement);
        return NULL;
    }
    token_free(g_list_pop(tokens));

    //Next token must be int
    if(((Token*)g_list_first(*tokens)->data)==NULL || ((Token*)g_list_first(*tokens)->data)->type != INT){
        statement_free(statement);
        return NULL;
    }
    Token *token = (Token*)g_list_pop(tokens);
    statement->limit = *(int*)token->data;
    token_free(token);

    if(g_list_length(*tokens)!=0){
        statement_free(statement);
        return NULL;
    }

    return statement;
}

//Function to parse the order by clause of a query
Statement *parse_order_clause(GList **tokens,Statement *statement) {
    if(g_list_length(*tokens)==0 || ((Token*)g_list_first(*tokens)->data)->type != ORDER){
        statement->order = NULL;
        return statement;
    }
    token_free(g_list_pop(tokens));

    //Next token must be by
    if(((Token*)g_list_first(*tokens)->data)==NULL || ((Token*)g_list_first(*tokens)->data)->type != BY){
        statement_free(statement);
        return NULL;
    }
    token_free(g_list_pop(tokens));

    //Next token is the group by var
    Token *token = (Token*)g_list_pop(tokens);
    Var *var = NULL;
    int isAsc=0;
    if(token->type==VAR){
        var = malloc(sizeof(Var));
        var->type = VAR;
        char *data = strdup((char*)token->data);
        char *t = strtok(data,".");
        var->table = strdup(t);
        t = strtok(NULL,".");
        var->column = strdup(t);
        free(data);
    }
    else if(token->type==COUNT || token->type==SUM){
        if(g_list_length(*tokens)>=3) {

            Token *var_token = NULL;

            Token *lookahead = (Token *) g_list_first(*tokens)->data;
            if (lookahead->type == LEFT_PARENTHESIS) {
                lookahead = (Token *) g_list_pop(tokens);
                token_free(lookahead);

                lookahead = (Token *) g_list_first(*tokens)->data;
                if (lookahead->type == VAR) {
                    var_token = (Token *) g_list_pop(tokens);

                    lookahead = (Token *) g_list_first(*tokens)->data;
                    if (lookahead->type == RIGHT_PARENTHESIS) {
                        lookahead = (Token *) g_list_pop(tokens);
                        token_free(lookahead);

                        var = malloc(sizeof(Var));
                        var->type = token->type;
                        char *data = strdup((char *) var_token->data);
                        char *t = strtok(data, ".");
                        var->table = strdup(t);
                        t = strtok(NULL, ".");
                        var->column = strdup(t);

                        free(data);
                    }
                }
            }
            if(var_token!=NULL)
                token_free(var_token);
        }
    }
    token_free(token);

    //Error in parsing
    if(var==NULL){
        statement_free(statement);
        return NULL;
    }
    else{
        if((Token*)g_list_first(*tokens)!=NULL){
            Token *last = (Token*)g_list_first(*tokens)->data;
            if(last->type==ASC){
                token_free(g_list_pop(tokens));
                isAsc = 1;
            }
            else if(last->type==DESC){
                token_free(g_list_pop(tokens));
                isAsc = 0;
            }
            else if(last->type!=LIMIT) {
                var_free(var);
                statement_free(statement);
                return NULL;
            }
        }
    }

    Order *order = malloc(sizeof(Order));
    order->var = var;
    order->isAsc = isAsc;

    statement->order = order;
    return statement;
}

//Function to parse the group by clause of a query
Statement *parse_group_clause(GList **tokens,Statement *statement){
    if(g_list_length(*tokens)==0 || ((Token*)g_list_first(*tokens)->data)->type != GROUP){
        statement->group = NULL;
        return statement;
    }
    token_free(g_list_pop(tokens));

    //Next token must be by
    if(((Token*)g_list_first(*tokens)->data)==NULL || ((Token*)g_list_first(*tokens)->data)->type != BY){
        statement_free(statement);
        return NULL;
    }
    token_free(g_list_pop(tokens));

    //Next token is the group by var
    Token *token = (Token*)g_list_pop(tokens);
    Var *var = NULL;
    if(token->type==VAR){
        var = malloc(sizeof(Var));
        var->type = VAR;
        char *data = strdup((char*)token->data);
        char *t = strtok(data,".");
        var->table = strdup(t);
        t = strtok(NULL,".");
        var->column = strdup(t);
        free(data);
    }
    token_free(token);

    //Error in parsing
    if(var==NULL){
        statement_free(statement);
        return NULL;
    }
    else{
        statement->group = var;
        if((Token*)g_list_first(*tokens)!=NULL){
            Token *last = (Token*)g_list_first(*tokens)->data;
            if(last->type!=ORDER && last->type!=LIMIT) {
                statement_free(statement);
                return NULL;
            }
        }
    }

    return statement;
}

//Function to parse the where clause of a query
Statement *parse_where_clause(GList **tokens,Statement *statement){
    if(g_list_length(*tokens)==0 || ((Token*)g_list_first(*tokens)->data)->type != WHERE){
        statement->expr=NULL;
        return statement;
    }
    token_free(g_list_pop(tokens));

    //Parse the boolean expression
    GList *expressions = NULL;

    while(1){
        if(g_list_length(*tokens)==0){
            statement_free(statement);
            g_list_free_full(expressions,expr_free);
            return NULL;
        }

        Token *token = (Token*)g_list_first(*tokens)->data;
        Expr *expr = NULL;
        if(token->type==VAR || token->type==INT || token->type==DOUBLE || token->type==STRING){
            expr = parse_expr(tokens);
        }

        if(expr==NULL){
            free(statement);
            g_list_free_full(expressions,expr_free);
            return NULL;
        }
        else{
            expressions = g_list_append(expressions,expr);
            if((Token*)g_list_first(*tokens)==NULL)
                break;

            Token *last = (Token*)g_list_first(*tokens)->data;
            if(last->type==GROUP || last->type==ORDER || last->type==LIMIT)
                break;
            else if(last->type==AND){
                last = (Token*)g_list_pop(tokens);
                token_free(last);
            }
            else{
                statement_free(statement);
                g_list_free_full(expressions,expr_free);
                return NULL;
            }
        }
    }

    BooleanExpr *booleanExpr = malloc(sizeof(BooleanExpr));
    booleanExpr->size = g_list_length(expressions);
    booleanExpr->expr = malloc(sizeof(Expr)*booleanExpr->size);
    int i=0;
    while(g_list_length(expressions)>0){
        booleanExpr->expr[i] = (Expr*) g_list_pop(&expressions);
        i++;
    }

    statement->expr = booleanExpr;
    return statement;
}

//Function to parse a list of tables (FROM clause)
Statement *parse_table_list(GList **tokens,Statement *statement){

    //First token must be FROM
    Token *token = (Token*)g_list_pop(tokens);
    if(token->type!=FROM){
        token_free(token);
        return NULL;
    }
    token_free(token);


    while(1){
        if(g_list_length(*tokens)==0){
            statement_free(statement);
            return NULL;
        }

        Token *token = (Token*)g_list_pop(tokens);
        TableToken *table = NULL;
        if(token->type==TABLE){
            table = parse_table(token,tokens);
        }

        //Error in parsing
        if(table==NULL){
            statement_free(statement);
            return NULL;
        }
        else{
            statement->tables = g_list_append(statement->tables,table);
            if((Token*)g_list_first(*tokens)==NULL)
                break;

            Token *last = (Token*)g_list_first(*tokens)->data;
            if(last->type==WHERE || last->type==GROUP || last->type==ORDER || last->type==LIMIT)
                break;
            else if(last->type==COMMA){
                last = (Token*)g_list_pop(tokens);
                token_free(last);
            }
            else{
                statement_free(statement);
                return NULL;
            }
        }
    }
    return statement;
}

//Function to parse a list of result variables
Statement *parse_var_list(GList **tokens,Statement *statement){
    while(1){
        if(g_list_length(*tokens)==0){
            statement_free(statement);
            return NULL;
        }

        Token *token = (Token*)g_list_pop(tokens);
        Var *var = NULL;
        if(token->type==VAR){
            var = parse_var(token,tokens);
        }
        else if(token->type==COUNT || token->type==SUM){
            var = parse_aggr(token,tokens);
        }

        //Error in parsing
        if(var==NULL){
            statement_free(statement);
            return NULL;
        }
        else{
            Token *last = (Token*)g_list_first(*tokens)->data;
            statement->vars = g_list_append(statement->vars,var);
            if(last->type==FROM)
                break;
            else if(last->type==COMMA){
                last = (Token*)g_list_pop(tokens);
                token_free(last);
            }
            else{
                statement_free(statement);
                return NULL;
            }
        }
    }

    return statement;
}




/********** SINGLE TOKEN PARSERS **************************/

//Function to parse an expr of the form a op b
Expr *parse_expr(GList **tokens){
    if(g_list_length(*tokens)<3)
        return NULL;
    if(isComparisonOperator((Token*)g_list_nth_data(*tokens,1))==0)
        return NULL;
    Token *third = (Token*)g_list_nth_data(*tokens,2);
    if(third->type!=VAR && third->type!=INT && third->type!=DOUBLE && third->type!=STRING)
        return NULL;

    Token *x = (Token*)g_list_pop(tokens);
    Token *op = (Token*)g_list_pop(tokens);
    Token *y = (Token*)g_list_pop(tokens);
    Expr *expr = malloc(sizeof(Expr));

    TokenType x_type,y_type;
    void *_x = parse_expr_operand(x,&x_type);
    void *_y = parse_expr_operand(y,&y_type);

    expr->x_type = x_type;
    expr->x = _x;
    expr->y_type = y_type;
    expr->y = _y;
    expr->op = op->type;

    token_free(x);
    token_free(op);
    token_free(y);
    return expr;
}

//Function to parse a single table
TableToken *parse_table(Token *table_token,GList **tokens){

    if(g_list_length(*tokens)!=0) {
        Token *lookahead = (Token *) g_list_first(*tokens)->data;
        if (lookahead->type != COMMA && lookahead->type != WHERE
            && lookahead->type != GROUP && lookahead->type != ORDER && lookahead->type != LIMIT) {
            token_free(table_token);
            return NULL;
        }
    }

    TableToken *table = table_token_create(table_token->data);
    token_free(table_token);
    return table;
}

//Function to parse a single var
Var *parse_var(Token *var_token,GList **tokens){
    if(g_list_length(*tokens)==0) {
        token_free(var_token);
        return NULL;
    }

    Token *lookahead = (Token*)g_list_first(*tokens)->data;
    if(lookahead->type!=COMMA && lookahead->type!=FROM) {
        token_free(var_token);
        return NULL;
    }

    Var *var = malloc(sizeof(Var));
    var->type = VAR;
    char *data = strdup((char*)var_token->data);
    char *t = strtok(data,".");
    var->table = strdup(t);
    t = strtok(NULL,".");
    var->column = strdup(t);

    free(data);
    token_free(var_token);
    return var;
}

//Function to parse a single aggregate
Var *parse_aggr(Token *aggr_token,GList **tokens){
    if(g_list_length(*tokens)<4) {
        token_free(aggr_token);
        return NULL;
    }

    Token *var_token = NULL;

    Token *lookahead = (Token*)g_list_first(*tokens)->data;
    if(lookahead->type != LEFT_PARENTHESIS){
        token_free(aggr_token);
        return NULL;
    }
    lookahead = (Token*)g_list_pop(tokens);
    token_free(lookahead);

    lookahead = (Token*)g_list_first(*tokens)->data;
    if(lookahead->type != VAR) {
        token_free(aggr_token);
        return NULL;
    }
    var_token = (Token*)g_list_pop(tokens);

    lookahead = (Token*)g_list_first(*tokens)->data;
    if(lookahead->type != RIGHT_PARENTHESIS) {
        token_free(var_token);
        token_free(aggr_token);
        return NULL;
    }
    lookahead = (Token*)g_list_pop(tokens);
    token_free(lookahead);

    lookahead = (Token*)g_list_first(*tokens)->data;
    if(lookahead==NULL || (lookahead->type!=COMMA && lookahead->type!=FROM)) {
        token_free(var_token);
        token_free(aggr_token);
        return NULL;
    }

    Var *var = malloc(sizeof(Var));
    var->type = aggr_token->type;
    char *data = strdup((char*)var_token->data);
    char *t = strtok(data,".");
    var->table = strdup(t);
    t = strtok(NULL,".");
    var->column = strdup(t);

    free(data);
    token_free(var_token);
    token_free(aggr_token);
    return var;
}


/************ HELPER FUNCTIONS *********************/
//Function to check if a token is a comparison operator
int isComparisonOperator(Token *token){
    if(token->type==EQUAL || token->type==NOT_EQUAL || token->type==GREATER || token->type==LESS
        || token->type==LEQ || token->type==GEQ) return 1;
    return 0;
}

//Create expression operand
void *parse_expr_operand(Token *token,TokenType *type){
    *type = token->type;
    if(token->type==VAR){
        Var *var = malloc(sizeof(Var));
        var->type = VAR;
        char *data = strdup((char*)token->data);
        char *t = strtok(data,".");
        var->table = strdup(t);
        t = strtok(NULL,".");
        var->column = strdup(t);
        free(data);
        return var;
    }
    else if(token->type==INT){
        int *value = malloc(sizeof(int));
        *value = *(int*)token->data;
        return value;
    }
    else if(token->type==DOUBLE){
        double *value = malloc(sizeof(double));
        *value = *(double*)token->data;
        return value;
    }
    else{
        char *value = strdup(token->data);
        return value;
    }
}


TableToken *table_token_create(char *name){
    TableToken *table = malloc(sizeof(TableToken));
    table->name = strdup(name);
    return table;
}

//Function to print an expr
void expr_print(Expr *expr){
    if(expr->x_type==VAR){
        Var *var = (Var*)expr->x;
        printf("%s.%s",var->table,var->column);
    }
    else if(expr->x_type==INT){
        printf("%d",*(int*)expr->x);
    }
    else if(expr->x_type==DOUBLE){
        printf("%f",*(double*)expr->x);
    }
    else{
        printf("'%s'",(char*)expr->x);
    }
    if(expr->op==EQUAL) printf("=");
    else if(expr->op==NOT_EQUAL) printf("!=");
    else if(expr->op==GREATER) printf(">");
    else if(expr->op==LESS) printf("<");
    else if(expr->op==GEQ) printf(">=");
    else if(expr->op==LEQ) printf("<=");
    if(expr->y_type==VAR){
        Var *var = (Var*)expr->y;
        printf("%s.%s",var->table,var->column);
    }
    else if(expr->y_type==INT){
        printf("%d",*(int*)expr->y);
    }
    else if(expr->y_type==DOUBLE){
        printf("%f",*(double*)expr->y);
    }
    else{
        printf("'%s'",(char*)expr->y);
    }
}

//Function to print a statement
void statement_print(Statement *statement){
    if(statement->type==SELECT_STMT){
        printf("\nSELECT ");
        for(int i=0;i<g_list_length(statement->vars);i++){
            var_print(g_list_nth(statement->vars,i)->data,NULL);
            if(i+1<g_list_length(statement->vars))
                printf(",");
        }
        printf("\n");
        printf("FROM ");
        for(int i=0;i<g_list_length(statement->tables);i++){
            table_print(g_list_nth(statement->tables,i)->data,NULL);
            if(i+1<g_list_length(statement->tables))
                printf(",");
        }
        printf("\n");
        if(statement->expr!=NULL){
            printf("WHERE ");
            for(int i=0;i<statement->expr->size;i++){
                expr_print(statement->expr->expr[i]);
                if(i+1<statement->expr->size)
                    printf(" AND ");
            }
            printf("\n");
        }
        if(statement->group!=NULL){
            printf("GROUP BY ");
            var_print(statement->group,NULL);
            printf("\n");
        }
        if(statement->order!=NULL){
            printf("ORDER BY ");
            var_print(statement->order->var,NULL);
            if(statement->order->isAsc==1)
                printf("ASC ");
            printf("\n");
        }
        if(statement->limit!=0){
            printf("LIMIT %d\n",statement->limit);
        }
        printf("\n");
    }
}

//Function to print a var
void var_print(void *_var,void *extra){
    Var *var = (Var*)_var;
    if(var->type==VAR)
        printf("%s.%s ",var->table,var->column);
    else{
        if(var->type==SUM)
            printf("SUM(%s.%s) ",var->table,var->column);
        else if(var->type==COUNT)
            printf("COUNT(%s.%s) ",var->table,var->column);
    }
}

//Function to print a table
void table_print(void *_table,void *extra){
    TableToken *table = (TableToken*)_table;
    printf("%s ",table->name);
}

//Function to free a statement
void statement_free(Statement *statement){
    if(statement->vars!=NULL)
        g_list_free_full(statement->vars,var_free);

    if(statement->tables!=NULL)
        g_list_free_full(statement->tables,table_free);

    if(statement->expr!=NULL)
        boolean_expr_free(statement->expr);

    if(statement->group!=NULL)
        var_free(statement->group);

    if(statement->order!=NULL){
        var_free(statement->order->var);
        free(statement->order);
    }

    free(statement);
}

//Function to free a boolean expr
void boolean_expr_free(BooleanExpr *expr){
    for(int i=0;i<expr->size;i++){
        expr_free(expr->expr[i]);
    }
    free(expr->expr);
    free(expr);
}

//Function to free an expr
void expr_free(void *_expr){
    Expr *expr = (Expr*)_expr;
    if(expr->x_type==VAR) var_free(expr->x);
    else free(expr->x);
    if(expr->y_type==VAR) var_free(expr->y);
    else free(expr->y);
    free(expr);
}

//Function to free a var
void var_free(void *var){
    Var *_var = (Var*)var;
    free(_var->table);
    free(_var->column);
    free(_var);
}

//Function to free a table
void table_free(void *table){
    TableToken *_table = (TableToken*)table;
    free(_table->name);
    free(_table);
}