#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record.h"
#include "result_set.h"

//Function to create a result set
ResultSet *result_set_create(){
    ResultSet *set = malloc(sizeof(ResultSet));
    set->tables=NULL;
    set->results=NULL;
    return set;
}

//Function to initialize a result item
ResultItem *result_item_create(){
    ResultItem *item = malloc(sizeof(ResultItem));
    item->size=0;
    item->records=NULL;
    return item;
}

//Function to add a table to the result set
ResultSet *result_set_add_table(ResultSet *set,Table *table){
    if(g_list_find(set->tables,table)==NULL)
        set->tables = g_list_append(table);
    return set;
}

//Function to add a table to the result set
ResultSet *result_set_add_item(ResultSet *set,ResultItem *item){
    set->results = g_list_append(set->results,item);
    return set;
}

//Function to add a record for a different table to the item
ResultItem *result_item_add_record(ResultItem *item,Record *record){
    item->records = g_list_append(item,record);
    return item;
}

