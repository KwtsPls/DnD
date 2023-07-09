#ifndef DND_RESULT_SET_H
#define DND_RESULT_SET_H

#include <glib.h>

typedef struct result_stream{
    GList *records;
}ResultItem;

typedef struct result_set{
    GList *results;
    GList *tables;
}ResultSet;

//Function to create a result set
ResultSet *result_set_create();
//Function to initialize a result item
ResultItem *result_item_create();
//Function to add a table to the result set
ResultSet *result_set_add_table(ResultSet *set,Table *table);
//Function to add a table to the result set
ResultSet *result_set_add_item(ResultSet *set,ResultItem *item);
//Function to add a record for a different table to the item
ResultItem *result_item_add_record(ResultItem *item,Record *record);


#endif //DND_RESULT_SET_H
