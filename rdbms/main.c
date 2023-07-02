#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./compiler/tokenizer.h"

int main(){
    GList *tokens = tokenize("SEleCT");
    printf("\nTOKENS: %d\n",g_list_length(tokens));
    print_all_tokens(tokens);

    g_list_free_full(tokens,token_free);
}

