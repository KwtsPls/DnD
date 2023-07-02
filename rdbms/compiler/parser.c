#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "tokenizer.h"

//Parse a list of tokens and convert it to a statement
Statement *parse_statement(GList *tokens){
    Statement *statement = malloc(sizeof(Statement));
    statement->tables=NULL;
    statement->vars=NULL;

    //pop first element of the list
    Token *token = (Token *)g_list_delete_link(tokens,tokens)->data;
    if(token->type==SELECT){
        statement = parse_select(tokens,statement);
    }
    else{
        printf("Parse error. Expected SELECT not ");
        token_print(token,NULL);
        printf("\n");
        free(statement);
        return NULL;
    }
}

//Function to parse select statements
Statement *parse_select(GList *tokens,Statement *statement){
    statement = parse_var_list(tokens,statement);
}

//Function to parse a list of result variables
Statement *parse_var_list(GList *tokens,Statement *statement){
    while(1){
        Token *token = (Token *)g_list_delete_link(tokens,tokens)->data;
        Var *var = NULL;
        if(token->type==VAR){
            var = parse_var(token,tokens,statement);
        }
        else if(token->type==COUNT || token->type==SUM){
            var = parse_aggr(token,tokens,statement);
        }
        else

    }
}

//Function to parse a single var
Var *parse_var(Token *var_token,GList *tokens,Statement *statement){
    Token *lookahead = (Token*)g_list_first(tokens);
    if(lookahead->type!=COMMA && lookahead->type!=FROM)
        return NULL;
    Token *next = (Token *)g_list_delete_link(tokens,tokens)->data;
    Var *var = malloc(sizeof(Var));
    var->type = VAR;
    char *data = strdup((char*)var_token->data);
    char *t = strtok(data,".");
    var->table = strdup(t);
    t = strtok(data,NULL);
    var->column = strdup(t);

    free(data);
    token_free(var_token);
    token_free(next);
    return var;
}

//Function to parse a single aggregate
Var *parse_aggr(Token *aggr_token,GList *tokens,Statement *statement){
    Token *var_token = NULL;

    Token *lookahead = (Token*)g_list_first(tokens);
    if(lookahead->type != LEFT_PARENTHESIS) return NULL;
    lookahead = (Token *)g_list_delete_link(tokens,tokens)->data;
    token_free(lookahead);

    lookahead = (Token*)g_list_first(tokens);
    if(lookahead->type != VAR) return NULL;
    var_token = lookahead;

    Token *lookahead = (Token*)g_list_first(tokens);
    if(lookahead->type != RIGHT_PARENTHESIS) {
        token_free(var_token);
        return NULL;
    }
    lookahead = (Token *)g_list_delete_link(tokens,tokens)->data;
    token_free(lookahead);

    lookahead = (Token*)g_list_first(tokens);
    if(lookahead->type!=COMMA && lookahead->type!=FROM)
        return NULL;
    Token *next = (Token *)g_list_delete_link(tokens,tokens)->data;

    Var *var = malloc(sizeof(Var));
    var->type = aggr_token->type;
    char *data = strdup((char*)var_token->data);
    char *t = strtok(data,".");
    var->table = strdup(t);
    t = strtok(data,NULL);
    var->column = strdup(t);

    free(data);
    token_free(var_token);
    token_free(aggr_token);
    token_free(next);
    return var;
}