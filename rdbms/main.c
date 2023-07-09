#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./disk/block.h"
#include "./memory/priority_queue.h"
#include "./memory/buffer_manager.h"
#include "./table/databox.h"
#include "./table/record.h"
#include "./db_files/heapfile.h"
#include "./db_files/result_file.h"

int main(){
    BlockAllocator *allocator = block_allocator_initialize(BUFFER_SIZE*1024);
    Table *table=NULL;

    GList *fields = NULL;
    GList *field_names = NULL;
    int *i = malloc(sizeof(int));
    *i = 125;
    DataBox *databox1 = databox_create(i, sizeof(int), INT_BOX);
    fields = g_list_append(fields,databox1);
    DataBox *databox2 = databox_create(strdup("asf12"), 50, STRING_BOX);
    fields = g_list_append(fields,databox2);
    DataBox *databox3 = databox_create(strdup("asf12"), 60, STRING_BOX);
    fields = g_list_append(fields,databox3);

    char *f = strdup("id");
    field_names = g_list_append(field_names,f);
    f = strdup("name");
    field_names = g_list_append(field_names,f);
    f = strdup("surname");
    field_names = g_list_append(field_names,f);

    heap_file_create(&allocator,"000","customer",fields,field_names);

    //open heap file
    int fd;
    HPErrorCode status = heap_file_open(&allocator,"000",&fd,&table);
    if(status==HP_FILE_ERROR) printf("INCORRECT FILE\n");

    //Insert 100 records as a test
    for(int i=0;i<100;i++){
        Record *r = record_create();

        int *data = malloc(sizeof(int));
        if(i%10==0) *data = 157;
        else *data=i;
        DataBox *databox = databox_create(data, sizeof(int),INT_BOX);
        r = record_add_field(r,databox);

        char *data_str = malloc(50);
        memset(data_str,0,50);
        strcpy(data_str,"HEAP-TEST-1");
        databox = databox_create(data_str,50,STRING_BOX);
        r = record_add_field(r,databox);

        data_str = malloc(60);
        memset(data_str,0,60);
        strcpy(data_str,"SECOND-TEST-2");
        databox = databox_create(data_str,60,STRING_BOX);
        r = record_add_field(r,databox);

        heap_file_insert(&allocator,r,fd);
        record_destroy(r);
    }

    //heap_file_print_all(&allocator,fd,fields,50+60+sizeof(int));
    int *value = malloc(sizeof(int));
    *value = 157;
    int rfd=-1;
    status = heap_file_filter(&allocator,&table,"id",EQUAL,value,0,&rfd);
    free(value);

    if(status==HP_FILTER_FIELD_ERROR) printf("FILTER FIELD ERROR\n");
    heap_file_print_all(&allocator,rfd,table->fields,table->record_size);

    result_file_close(&allocator,rfd);
    heap_file_close(&allocator,fd,&table);
    block_allocator_destroy(allocator);
    g_list_free_full(fields, databox_destroy);
    g_list_free_full(field_names, free);
    return 0;
}

