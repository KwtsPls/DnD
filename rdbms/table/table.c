#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

//Function to initialize a table
Table *table_init(){
    Table *table = malloc(sizeof(Table));
    table->filename=NULL;
    table->metadata=NULL;
    table->fd=-1;
    table->fields=NULL;
    table->field_names=NULL;
    table->name=NULL;
    table->record_size=0;
    return table;
}

//Function to get the pos of a field
int table_field_pos(Table *table,char *field_name){
    GList *field_names = table->field_names;
    for(int i=0;i<g_list_length(field_names);i++){
        char *field = (char*)g_list_nth(field_names,i)->data;
        if(strcmp(field,field_name)==0) return i;
    }
    return -1;
}

//Helper function for results
int result_condition(int result,TokenType op){
    if(op==EQUAL)
        return (result==0)?1:0;
    else if(op==NOT_EQUAL)
        return (result!=0)?1:0;
    else if(op==GREATER)
        return (result>0)?1:0;
    else if(op==LESS)
        return (result<0)?1:0;
    else if(op==GEQ)
        return (result>=0)?1:0;
    else
        return (result<=0)?1:0;
}

//Function to deallocate a table
void table_destroy(void *_table){
    Table *table = (Table*)_table;
    free(table->filename);
    free(table->metadata);
    free(table->name);
    g_list_free_full(table->fields, free);
    g_list_free_full(table->field_names, free);
    free(table);
}
