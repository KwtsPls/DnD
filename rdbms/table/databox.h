#ifndef DND_DATABOX_H
#define DND_DATABOX_H

typedef enum datatype{INT_BOX,DOUBLE_BOX,STRING_BOX}DataType;

/* A simple abstraction to implement the various datatypes*/
typedef struct databox{
    void *data;
    int size;
    DataType type;
}DataBox;

//Function to create a new databox
DataBox *databox_create(void *data,int size,DataType type);
//Get data if databox holds an integer
int databox_get_int(DataBox *databox);
//Get data if databox holds a double
double databox_get_double(DataBox *databox);
//Get data if databox holds a string
char *databox_get_string(DataBox *databox);
//Compare a value with the data held in a databox
int databox_compare_value(DataBox *databox,void *value);
//Compare two databoxes
int databox_compare(DataBox *databox1,DataBox *databox2);
//Function make a deep copy of a databox
void *databox_copy(const void *_databox,void *extra);
//Function to sum on a databoxes
DataBox *databox_sum(DataBox *databox,DataBox *sum);
//Function to count on a data box
DataBox *databox_count(DataBox *databox);
//Function to destroy a databox
void databox_destroy(void *_databox);

#endif //DND_DATABOX_H
