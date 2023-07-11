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
    if(databox->type==INT_BOX) return *(int*)databox->data - *(int*)value;
    else if(databox->type==DOUBLE_BOX) return *(double*)databox->data - *(double*)value;
    else return strcmp((char*)databox->data,(char*)value);
}

//Compare two databoxes
int databox_compare(DataBox *databox1,DataBox *databox2){
    if(databox1->type==INT_BOX && databox2->type==INT_BOX) return *(int*)databox1->data - *(int*)databox2->data;
    else if(databox1->type==DOUBLE_BOX && databox2->type==DOUBLE_BOX) return *(double*)databox1->data - *(double*)databox2->data;
    else if (databox1->type==STRING_BOX && databox2->type==STRING_BOX) return strcmp((char*)databox1->data,(char*)databox2->data);
    else return INT_MIN;
}

//Function to count on a data box
DataBox *databox_count(DataBox *databox){
    if(databox==NULL){
        int *value = malloc(sizeof(int));
        *value=1;
        databox = databox_create(value,sizeof(int),INT_BOX);
        return databox;
    }

    int value = *(int*)databox->data;
    value++;
    *(int*)databox->data = value;
    return databox;
}

//Function to sum on a databoxes
DataBox *databox_sum(DataBox *databox,DataBox *sum){
    if(databox==NULL){
        if(sum->type==INT_BOX){
            int *value = malloc(sizeof(int));
            *value = *(int*)sum->data;
            databox = databox_create(value,sum->size,sum->type);
        }
        else{
            double *value = malloc(sizeof(double));
            *value = *(double*)sum->data;
            databox = databox_create(value,sum->size,sum->type);
        }
        return databox;
    }

    if(databox->type==INT_BOX){
        *(int*)databox->data = *(int*)databox->data + *(int*)sum->data;
    }
    else{
        *(double*)databox->data = *(double*)databox->data + *(double*)sum->data;
    }
    return databox;
}

//Function make a deep copy of a databox
void *databox_copy(const void *_databox,void *extra){
    DataBox *databox = (DataBox*)_databox;
    DataBox *copy = malloc(sizeof(DataBox));
    copy->size = databox->size;
    copy->type = databox->type;
    if(databox->type==INT_BOX){
        int *data = malloc(sizeof(int));
        *data = *(int*)databox->data;
        copy->data = data;
    }
    else if(databox->type==DOUBLE_BOX){
        double *data = malloc(sizeof(double));
        *data = *(double*)databox->data;
        copy->data = data;
    }
    else{
        char *data = malloc(databox->size);
        memset(data,0,databox->size);
        strcpy(data,(char*)databox->data);
        copy->data = data;
    }
    return copy;
}

//Function to destroy a databox
void databox_destroy(void *_databox){
    DataBox *databox = (DataBox*) _databox;
    free(databox->data);
    free(databox);
}

