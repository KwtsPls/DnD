#ifndef DND_TOKENIZER_H
#define DND_TOKENIZER_H

#include <glib.h>

typedef enum {TOKENIZER_OK, TOKENIZER_FAILURE}TokenizerResult;
typedef enum {INT,DOUBLE,VAR,TABLE,STRING,SELECT,INSERT,WHERE,FROM,DISTINCT,LEFT_PARENTHESIS,
                RIGHT_PARENTHESIS,GREATER,LESS,GEQ,LEQ,EQUAL,NOT_EQUAL,ADD,SUBTRACT,MULTIPLY,DIVIDE,
                AND,OR,GROUP,ORDER,BY,COUNT,SUM,LIMIT,COMMA,ASC,DESC}TokenType;

typedef enum {VARIABLE,TABLE_NAME}TokenizerIndex;

typedef struct token{
    void *data;
    TokenType type;
}Token;

//Function to tokenize a given query - a list of tokens in returned
GList *tokenize(char *query);

//Tokenizers
GList *tokenizeWord(GList *l,char buffer[],char *query,int *i);
GList *tokenizeOneByteOperator(GList *l,char byte,int *i);
GList *tokenizeTwoByteOperator(GList *l,char *query,int *i);
GList *tokenizeString(GList *l,char buffer[],char *query,int *i);
GList *tokenizeNumber(GList *l,char buffer[],char *query,int *i);


//Function to delete a token
void token_free(void *);
//Function to print a token
void token_print(void *data,void *extra);

//Function to print a list of tokens
void print_all_tokens(GList *tokens);

//Helper functions
int isLetter(char c);
int isOperator(char c);
int isNumber(char c);
int isWhitespace(char c);

//Helper keyword functions
TokenType isKeyword(char buffer[]);
int isSelectKeyword(char buffer[]);
int isInsertKeyword(char buffer[]);
int isWhereKeyword(char buffer[]);
int isFromKeyword(char buffer[]);
int isAndKeyword(char buffer[]);
int isOrKeyword(char buffer[]);
int isCountKeyword(char buffer[]);
int isSumKeyword(char buffer[]);
int isGroupKeyword(char buffer[]);
int isOrderKeyword(char buffer[]);
int isByKeyword(char buffer[]);
int isDescKeyword(char buffer[]);
int isAscKeyword(char buffer[]);
int isLimitKeyword(char buffer[]);

#endif //DND_TOKENIZER_H
