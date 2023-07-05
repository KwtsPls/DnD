#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"

int main(){
    GList *tokens = tokenize("SEleCT Table.id,Table.salary FROM Table,Tabas,Taboo WHERE Table.id = 5 AND Table.salary = 'aeijfas'"
                             " ORDER BY COUNT(Table.id) ASC LIMIT 6 1");
    printf("\nTOKENS: %d\n",g_list_length(tokens));
    print_all_tokens(tokens);
    Statement *statement = parse_statement(&tokens);
    if(statement!=NULL)
        statement_print(statement);

    g_list_free_full(tokens,token_free);
    if(statement!=NULL)
        statement_free(statement);
}

