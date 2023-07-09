#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "databox.h"

//Function to create a new databox
DataBox *databox_create(void *data,int size,DataType type){
    DataBox *databox = malloc(sizeof(DataBox));
    databox->data = data;
    databox->type = type;
    databox->size = size;
    return databox;
}

//Get data if databox holds an integer
int databox_get_int(DataBox *databox){
    if(databox->type==INT_BOX) return *(int*)databox->data;
    else return INT_MIN;
}

//Get data if databox holds a double
double databox_get_double(DataBox *databox){
    if(databox->type==DOUBLE_BOX) return *(double*)databox->data;
    else return INT_MIN;
}

//Get data if databox holds a string
char *databox_get_string(DataBox *databox){
    if(databox->type==STRING_BOX) return (char*)databox->data;
    else return NULL;
}

//Compare a value with the data held in a databox
int databox_compare_value(DataBox *databox,void *value){
    if(databox->type==INT_MAX) return *(int*)databox->data - *(int*)value;
    else if(databox->type==DOUBLE_BOX) return *(double*)databox->data - *(double*)value;
    else return strcmp((char*)databox->data,(char*)value);
}

//Function to destroy a databox
void databox_destroy(void *_databox){
    DataBox *databox = (DataBox*) _databox;
    free(databox->data);
    free(databox);
}

