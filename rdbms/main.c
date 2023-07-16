#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./compiler/semantic.h"
#include "./disk/block.h"
#include "./db_files/heapfile.h"
#include "./db_files/db_file.h"

int main(int argc, char **argv){
    Database *database = database_open("./data/data1/");
    database = database_open_existing (database, "./data/data0/");
    database = database_open_existing (database, "./data/data2/");
    database = database_open_existing (database, "./data/data3/");
    GList *records = database_query(database,"SELECT customer.id FROM customer  WHERE customer.surname='Pesilda' LIMIT 10");
    for(GList *node = records; node != NULL; node = node->next){
        Record *r = (Record*)node->data;
        record_print(r);
        printf("\n");
    }

    g_list_free_full(records, record_destroy);
    database_close(database);
}