#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokenizer.h"

//Function to tokenize a given query - a list of tokens in returned
GList *tokenize(char *query){
    GList *tokens = NULL;
    char buffer[512];
    memset(buffer,0,512);

    int i=0;
    while(i<strlen(query)){
        char byte = query[i];

        if(isLetter(byte)==1){
            tokens = tokenizeWord(tokens,buffer,query,&i);
            memset(buffer,0,512);
        }
        else if(isOperator(byte)==1 || byte=='(' || byte==')' || byte==','){
            tokens = tokenizeOneByteOperator(tokens,byte,&i);
        }
        else if(byte=='!' || byte=='<' || byte=='>'){
            tokens = tokenizeTwoByteOperator(tokens,query,&i);
        }
        else if(byte=='\'' || byte=='\"'){
            tokens = tokenizeString(tokens,buffer,query,&i);
            memset(buffer,0,512);
        }
        else if(isNumber(byte)==1){
            tokens = tokenizeNumber(tokens,buffer,query,&i);
            memset(buffer,0,512);
        }
        else if(isWhitespace(byte)==1){
            i++;
        }
        else{
            g_list_free_full(tokens,token_free);
            return NULL;
        }

        if(tokens==NULL) return NULL;
    }

    return tokens;
}

/************** TOKENIZERS ***************************/
GList *tokenizeNumber(GList *l,char buffer[],char *query,int *i){
    TokenType type = INT;

    while(*i < strlen(query)){
        char byte = query[*i];

        if(isNumber(byte)==1)
            strncat(buffer,&byte,1);
        else if(byte=='.'){
            if(type==DOUBLE) {
                g_list_free_full(l,token_free);
                return NULL;
            }
            else{
                type=DOUBLE;
                strncat(buffer,&byte,1);
            }
        }
        else if(isWhitespace(byte)==1){
            break;
        }
        else{
            g_list_free_full(l,token_free);
            return NULL;
        }
        (*i)++;
    }

    Token *token = malloc(sizeof(Token));
    char *ptr;
    if(type==INT){
        int *i = malloc(sizeof(int));
        *i = strtol(buffer, &ptr, 10);
        token->data = i;
        token->type = type;
    }
    else{
        double *d = malloc(sizeof(double));
        *d = strtod(buffer,&ptr);
        token->data = d;
        token->type = type;
    }

    l = g_list_append(l,token);
    return l;
}

GList *tokenizeString(GList *l,char buffer[],char *query,int *i){
    char string_type;
    if(query[*i]=='\'') string_type = '\'';
    else string_type = '\"';

    (*i)++;
    while(*i < strlen(query)){
        char byte = query[*i];
        if(byte == string_type)
            break;

        strncat(buffer,&byte,1);
        (*i)++;
    }

    Token *token = NULL;
    if(*i == strlen(query) &&  query[*i-1]!=string_type){
        g_list_free_full(l,token_free);
        return NULL;
    }

    token = malloc(sizeof(Token));
    token->data = strdup(buffer);
    token->type = STRING;

    (*i)++;
    l = g_list_append(l,token);
    return l;
}

GList *tokenizeTwoByteOperator(GList *l,char *query,int *i){
    if(*i==strlen(query)){
        g_list_free_full(l,token_free);
        return NULL;
    }

    Token *token = NULL;
    if(query[*i]=='!'){
        (*i)++;
        if(query[*i]=='='){
            (*i)++;
            token = malloc(sizeof(Token));
            token->data=NULL;
            token->type=NOT_EQUAL;
        }
    }
    else if(query[*i]=='<'){
        (*i)++;
        if(query[*i]=='='){
            (*i)++;
            token = malloc(sizeof(Token));
            token->data=NULL;
            token->type=LEQ;
        }
        else{
            token = malloc(sizeof(Token));
            token->data=NULL;
            token->type=LESS;
        }
    }
    else if(query[*i]=='>'){
        (*i)++;
        if(query[*i]=='='){
            (*i)++;
            token = malloc(sizeof(Token));
            token->data=NULL;
            token->type=GEQ;
        }
        else{
            token = malloc(sizeof(Token));
            token->data=NULL;
            token->type=GREATER;
        }
    }

    if(token==NULL){
        g_list_free_full(l,token_free);
        return NULL;
    }

    l = g_list_append(l,token);
    return l;
}

GList *tokenizeOneByteOperator(GList *l,char byte,int *i){
    Token *token=malloc(sizeof(Token));
    token->data=NULL;
    if(byte=='+') token->type=ADD;
    else if(byte=='-') token->type=SUBTRACT;
    else if(byte=='*') token->type=MULTIPLY;
    else if(byte=='\\') token->type=DIVIDE;
    else if(byte=='(') token->type=LEFT_PARENTHESIS;
    else if(byte==')') token->type=RIGHT_PARENTHESIS;
    else if(byte==',') token->type=COMMA;
    else if(byte=='=') token->type=EQUAL;
    (*i)++;
    l = g_list_append(l,token);
    return l;
}

GList *tokenizeWord(GList *l,char buffer[],char *query,int *i){
    TokenizerIndex index = TABLE_NAME;

    while(*i < strlen(query)){
        char byte = query[*i];

        if(isLetter(byte)==1){
            strncat(buffer,&byte,1);
        }
        else if(byte=='.'){
            if(index==VARIABLE) {
                g_list_free_full(l,token_free);
                return NULL;
            }

            strncat(buffer,&byte,1);
            index = VARIABLE;
        }
        else if(isWhitespace(byte)==1 || byte==',' || byte=='(' ||
                    isOperator(byte) || byte=='<' || byte=='>' || byte=='!'){
            break;
        }
        else{
            g_list_free_full(l,token_free);
            return NULL;
        }

        (*i)++;
    }

    //table name or keyword
    Token *token = malloc(sizeof(Token));
    if(index==TABLE_NAME){
        TokenType type = isKeyword(buffer);
        if(type!=-1){
            token->data = NULL;
            token->type = type;
        }
        else{
            token->data = strdup(buffer);
            token->type = TABLE;
        }
    }
    else{
        token->data = strdup(buffer);
        token->type = VAR;
    }

    l = g_list_append(l,token);
    return l;
}

//Function to delete a token
void token_free(void *data){
    Token *token = (Token*)data;
    if(token->data!=NULL){
        free(token->data);
    }
    free(token);
}

//Function to print a token
void token_print(void *data,void *extra){
    Token  *token = (Token*)data;
    if(token->type==SELECT) printf("SELECT");
    else if(token->type==INSERT) printf("INSERT");
    else if(token->type==WHERE) printf("WHERE");
    else if(token->type==FROM) printf("FROM");
    else if(token->type==DISTINCT) printf("DISTINCT");
    else if(token->type==LEFT_PARENTHESIS) printf("(");
    else if(token->type==RIGHT_PARENTHESIS) printf(")");
    else if(token->type==EQUAL) printf("=");
    else if(token->type==NOT_EQUAL) printf("!=");
    else if(token->type==ADD) printf("+");
    else if(token->type==SUBTRACT) printf("-");
    else if(token->type==MULTIPLY) printf("*");
    else if(token->type==DIVIDE) printf("\\");
    else if(token->type==AND) printf("AND");
    else if(token->type==OR) printf("OR");
    else if(token->type==GROUP) printf("GROUP");
    else if(token->type==ORDER) printf("ORDER");
    else if(token->type==BY) printf("BY");
    else if(token->type==COUNT) printf("COUNT");
    else if(token->type==SUM) printf("SUM");
    else if(token->type==LIMIT) printf("LIMIT");
    else if(token->type==COMMA) printf("COMMA");
    else if(token->type==GREATER) printf(">");
    else if(token->type==LESS) printf("<");
    else if(token->type==GEQ) printf(">=");
    else if(token->type==LEQ) printf("<=");
    else if(token->type==INT){
        printf("Integer(%d)",*(int*)token->data);
    }
    else if(token->type==DOUBLE){
        printf("Double(%f)",*(double*)token->data);
    }
    else if(token->type==STRING){
        printf("String('%s')",(char*)token->data);
    }
    else if(token->type==TABLE){
        printf("Table(%s)",(char*)token->data);
    }
    else if(token->type==VAR){
        printf("Var(%s)",(char*)token->data);
    }
}

void print_all_tokens(GList *tokens){
    int n = g_list_length(tokens);
    printf("[");
    for(int i=0;i<n;i++){
        token_print(g_list_nth(tokens,i)->data,NULL);
        if(i+1<n)
            printf(",");
    }
    printf("]\n");
}

/***************** HELPER FUNCTIONS **************************/
int isLetter(char c){
    if( (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_') return 1;
    else return 0;
}

int isOperator(char c){
    if(c=='*' || c=='+' || c=='-' || c=='\\' || c=='=') return 1;
    else return 0;
}

int isNumber(char c){
    if(c>='0' && c<='9') return 1;
    else return 0;
}

int isWhitespace(char c){
    if(c==' ' || c=='\t' || c=='\n') return 1;
    else return 0;
}

/***************** Helper keyword functions **************************/
TokenType isKeyword(char buffer[]){
    if(isSelectKeyword(buffer)==1) return SELECT;
    else if(isInsertKeyword(buffer)==1) return INSERT;
    else if(isWhereKeyword(buffer)==1) return WHERE;
    else if(isFromKeyword(buffer)==1) return FROM;
    else if(isAndKeyword(buffer)==1) return AND;
    else if(isOrKeyword(buffer)==1) return OR;
    else if(isCountKeyword(buffer)==1) return COUNT;
    else if(isSumKeyword(buffer)==1) return SUM;
    else if(isGroupKeyword(buffer)==1) return GROUP;
    else if(isOrderKeyword(buffer)==1) return ORDER;
    else if(isByKeyword(buffer)==1) return BY;
    else return -1;
}

int isSelectKeyword(char buffer[]){
    if(strlen(buffer)!=6) return 0;
    if(buffer[0]!='S' && buffer[0]!='s') return 0;
    if(buffer[1]!='E' && buffer[1]!='e') return 0;
    if(buffer[2]!='L' && buffer[2]!='l') return 0;
    if(buffer[3]!='E' && buffer[3]!='e') return 0;
    if(buffer[4]!='C' && buffer[4]!='c') return 0;
    if(buffer[5]!='T' && buffer[5]!='t') return 0;
    return 1;
}

int isInsertKeyword(char buffer[]){
    if(strlen(buffer)!=6) return 0;
    if(buffer[0]!='I' && buffer[0]!='i') return 0;
    if(buffer[1]!='N' && buffer[1]!='n') return 0;
    if(buffer[2]!='S' && buffer[2]!='s') return 0;
    if(buffer[3]!='E' && buffer[3]!='e') return 0;
    if(buffer[4]!='R' && buffer[4]!='r') return 0;
    if(buffer[5]!='T' && buffer[5]!='t') return 0;
    return 1;
}

int isWhereKeyword(char buffer[]){
    if(strlen(buffer)!=5) return 0;
    if(buffer[0]!='W' && buffer[0]!='w') return 0;
    if(buffer[1]!='H' && buffer[1]!='h') return 0;
    if(buffer[2]!='E' && buffer[2]!='e') return 0;
    if(buffer[3]!='R' && buffer[3]!='r') return 0;
    if(buffer[4]!='E' && buffer[4]!='e') return 0;
    return 1;
}

int isFromKeyword(char buffer[]){
    if(strlen(buffer)!=4) return 0;
    if(buffer[0]!='F' && buffer[0]!='f') return 0;
    if(buffer[1]!='R' && buffer[1]!='r') return 0;
    if(buffer[2]!='O' && buffer[2]!='o') return 0;
    if(buffer[3]!='M' && buffer[3]!='m') return 0;
    return 1;
}

int isAndKeyword(char buffer[]){
    if(strlen(buffer)!=3) return 0;
    if(buffer[0]!='A' && buffer[0]!='a') return 0;
    if(buffer[1]!='N' && buffer[1]!='n') return 0;
    if(buffer[2]!='D' && buffer[2]!='d') return 0;
    return 1;
}

int isOrKeyword(char buffer[]){
    if(strlen(buffer)!=2) return 0;
    if(buffer[0]!='O' && buffer[0]!='o') return 0;
    if(buffer[1]!='R' && buffer[1]!='r') return 0;
    return 1;
}

int isCountKeyword(char buffer[]){
    if(strlen(buffer)!=5) return 0;
    if(buffer[0]!='C' && buffer[0]!='c') return 0;
    if(buffer[1]!='O' && buffer[1]!='o') return 0;
    if(buffer[2]!='U' && buffer[2]!='u') return 0;
    if(buffer[3]!='N' && buffer[3]!='n') return 0;
    if(buffer[4]!='T' && buffer[4]!='t') return 0;
    return 1;
}

int isSumKeyword(char buffer[]){
    if(strlen(buffer)!=3) return 0;
    if(buffer[0]!='S' && buffer[0]!='s') return 0;
    if(buffer[1]!='U' && buffer[1]!='u') return 0;
    if(buffer[2]!='M' && buffer[2]!='m') return 0;
    return 1;
}

int isGroupKeyword(char buffer[]){
    if(strlen(buffer)!=5) return 0;
    if(buffer[0]!='G' && buffer[0]!='g') return 0;
    if(buffer[1]!='R' && buffer[1]!='r') return 0;
    if(buffer[2]!='O' && buffer[2]!='o') return 0;
    if(buffer[3]!='U' && buffer[3]!='u') return 0;
    if(buffer[4]!='P' && buffer[4]!='p') return 0;
    return 1;
}

int isOrderKeyword(char buffer[]){
    if(strlen(buffer)!=5) return 0;
    if(buffer[0]!='O' && buffer[0]!='o') return 0;
    if(buffer[1]!='R' && buffer[1]!='r') return 0;
    if(buffer[2]!='D' && buffer[2]!='d') return 0;
    if(buffer[3]!='E' && buffer[3]!='e') return 0;
    if(buffer[4]!='R' && buffer[4]!='r') return 0;
    return 1;
}

int isByKeyword(char buffer[]){
    if(strlen(buffer)!=2) return 0;
    if(buffer[0]!='B' && buffer[0]!='b') return 0;
    if(buffer[1]!='Y' && buffer[1]!='y') return 0;
    return 1;
}