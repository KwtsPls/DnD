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

//Function to return the value of certain field
void *record_get_field_value(Record *r,int pos){
    int n = g_list_length(r->list);
    for(int i=0;i<n;i++){
        DataBox *databox = (DataBox*)g_list_nth(r->list,i)->data;
        if(pos==i)
            return databox;
    }
    return NULL;
}

//Function to compare two records
int record_compare(Record *r1,Record *r2){
    if(g_list_length(r1->list)!=g_list_length(r2->list)) return 0;
    if(r1->size != r2->size) return 0;

    for(int i=0;i<g_list_length(r1->list);i++){
        DataBox *databox1 = (DataBox*)g_list_nth(r1->list,i)->data;
        DataBox *databox2 = (DataBox*)g_list_nth(r2->list,i)->data;
        if(databox_compare(databox1,databox2)!=0) return 0;
    }

    return 1;
}

//Function to print a record
void record_print(Record *r){
    GString *s = record_to_string (r);
    printf("%s", s->str);
    g_string_free (s, TRUE);
}

//Function to print a record
GString* record_to_string(Record *r){
  GString *s = g_string_new ("");
  int n = g_list_length(r->list);
  for(int i=0;i<n;i++) {
      DataBox *databox = (DataBox*)g_list_nth(r->list,i)->data;
      if(databox->type==INT_BOX) g_string_append_printf(s, "%d ",*(int*)databox->data);
      else if (databox->type==DOUBLE_BOX) g_string_append_printf(s, "%f ",*(double *)databox->data);
      else g_string_append_printf(s, "%s ",(char *)databox->data);
    }
  g_string_append_printf(s, " ");
  return s;
}

//Function to sort a record list based on a field value
int record_compare_asc(const void *a,const void *b,void *extra){
    Record *ra = (Record*)a;
    Record *rb = (Record*)b;
    int i = *(int*)extra;
    DataBox *databoxa = record_get_field_value(ra,i);
    DataBox *databoxb = record_get_field_value(rb,i);
    return databox_compare(databoxa,databoxb);
}

//Function to sort a record list based on a field value
int record_compare_desc(const void *a,const void *b,void *extra){
    Record *ra = (Record*)a;
    Record *rb = (Record*)b;
    int i = *(int*)extra;
    DataBox *databoxa = record_get_field_value(ra,i);
    DataBox *databoxb = record_get_field_value(rb,i);
    return -databox_compare(databoxa,databoxb);
}

//Function to perform a deep copy of a record
void *record_copy(const void *_r,void *extra){
    Record *r = (Record*)_r;
    Record *copy = malloc(sizeof(Record));
    copy->size = r->size;
    copy->list = g_list_copy_deep(r->list,databox_copy,NULL);
    return copy;
}

//Function to destroy a record
void record_destroy(void *_r){
    if(_r==NULL) return;
    Record *r = (Record*)_r;
    g_list_free_full(r->list, databox_destroy);
    free(r);
}