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
//Function to print a record
void record_print(Record *r);
//Function to destroy a record
void record_destroy(Record *r);

#endif //DND_RECORD_H

