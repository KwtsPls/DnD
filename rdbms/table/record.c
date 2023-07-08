#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "record.h"

//Function to initialize a record
Record *record_create(){
    Record *r = malloc(sizeof(Record));
    r->size=0;
    r->list=NULL;
    return r;
}

//Function to add a field in the record
Record *record_add_field(Record *r,DataBox *databox){
    r->list = g_list_append(r->list,databox);
    r->size += databox->size;
    return r;
}

//Function to return the size of a record
int record_size(Record *r){
    return r->size;
}

//Function to return the fields of the record as an array
DataBox **record_as_array(Record *r,int *size){
    int n = g_list_length(r->list);
    DataBox **array = malloc(sizeof(DataBox*)*n);
    for(int i=0;i<n;i++){
        array[i] = (DataBox*)g_list_nth(r->list,i)->data;
    }
    *size = n;
    return array;
}

//Function to print a record
void record_print(Record *r){
    int n = g_list_length(r->list);
    for(int i=0;i<n;i++) {
        DataBox *databox = (DataBox*)g_list_nth(r->list,i)->data;
        if(databox->type==INT_BOX) printf("%d ",*(int*)databox->data);
        else if (databox->type==DOUBLE_BOX) printf("%f ",*(double *)databox->data);
        else printf("%s ",(char *)databox->data);
    }
    printf("\n");
}

//Function to destroy a record
void record_destroy(Record *r){
    g_list_free_full(r->list, databox_destroy);
    free(r);
}