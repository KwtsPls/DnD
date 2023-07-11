#ifndef DND_RECORD_H
#define DND_RECORD_H

#include <glib.h>
#include "databox.h"

//A record is a list of databoxes
typedef struct record{
    GList *list;
    int size;
}Record;

//Function to initialize a record
Record *record_create();
//Function to add a field in the record
Record *record_add_field(Record *r,DataBox *databox);
//Function to return the size of a record
int record_size(Record *r);
//Function to return the fields of the record as an array
DataBox **record_as_array(Record *r,int *size);
//Function to return the value of certain field
void *record_get_field_value(Record *r,int pos);
//Function to compare two records
int record_compare(Record *r1,Record *r2);
//Function to perform a deep copy of a record
void *record_copy(const void *_r,void *extra);
//Function to sort a record list based on a field value
int record_compare_asc(const void *a,const void *b,void *extra);
//Function to sort a record list based on a field value
int record_compare_desc(const void *a,const void *b,void *extra);
//Function to print a record
void record_print(Record *r);
//Function to destroy a record
void record_destroy(void *_r);

#endif //DND_RECORD_H

