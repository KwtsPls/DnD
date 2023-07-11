#ifndef DND_RESULT_SET_H
#define DND_RESULT_SET_H

#include <glib.h>
#include "../compiler/parser.h"
#include "../db_files/db_file.h"
#include "table.h"
#include "record.h"

typedef struct result_stream{
    GList *records;
}ResultItem;

typedef struct result_set{
    GList *results;
    GList *tables;
}ResultSet;

typedef struct result_item_entry{
    int i;
    int j;
    ResultItem *item;
}ResultEntry;

//Function to create a result entry
ResultEntry *result_entry_create(ResultItem *item,int i,int j);

//Function to create a result set
ResultSet *result_set_create();
//Function to initialize a result item
ResultItem *result_item_create();
//Function to add a table to the result set
ResultSet *result_set_add_table(ResultSet *set,Table *table);
//Function to add a table to the result set
ResultSet *result_set_add_item(ResultSet *set,ResultItem *item);
//Function to get the indexes (i,j) for a record and its field
void result_set_get_indexes(ResultSet *set,Table *table,char *field,int *i,int *j);
//Function to get the value of a record's field based on indexes
void *result_item_get_value(ResultItem *item,int i,int j);
//Function to add a record for a different table to the item
ResultItem *result_item_add_record(ResultItem *item,Record *record);
//Function to check if a record exists in a record list
int result_item_check_record(ResultItem *item,Record *r);
//Function to perform an intersection on two result items
ResultItem *result_item_intersection(ResultItem *item1,ResultItem *item2,GList *indexes);
//Function to perform an AND operation on two result sets
ResultSet *result_set_and(ResultSet *set1,ResultSet *set2);
//Function to insert the records of a result item to another
ResultItem *result_item_get_records(ResultItem *item1,ResultItem *item2);
//Function to prepare the result according to the variables in the select

/*********** OPERATIONS ON THE RESULT SET *****************************/
GList *result_set_finalize(ResultSet *set,Statement *statement,GList *db_files);

//Function to delete a result
void result_set_destroy(void *_set);
//Function to delete a result item
void result_item_destroy(void *_item);
//Function to delete a result set but not the records
void result_set_destroy_soft(void *_set);
//Function to delete a result set but not the records
void result_set_destroy_soft_1(void *_set);
//Function to delete a result item but not the records
void result_item_destroy_soft_1(void *_item);
//Function to delete a result item but not the records
void result_item_destroy_soft(void *_item);

#endif //DND_RESULT_SET_H
