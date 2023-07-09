#ifndef DND_TABLE_H
#define DND_TABLE_H

#include <glib.h>
#include "../compiler/tokenizer.h"

typedef struct table{
    char *name;
    char *filename;
    char *metadata;
    int fd;
    int record_size;
    GList *fields;
    GList *field_names;
}Table;

//Function to initialize a table
Table *table_init();
//Function to get the pos of a field
int table_field_pos(Table *table,char *field_name);
//Helper function for results
int result_condition(int result,TokenType op);
//Function to deallocate a table
void table_destroy(void *_table);

#endif //DND_TABLE_H
