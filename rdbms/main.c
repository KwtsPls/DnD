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

int main(){
    BlockAllocator *allocator = block_allocator_initialize(2048);

    GList *fields = NULL;
    int *i = malloc(sizeof(int));
    *i = 125;
    DataBox *databox1 = databox_create(i, sizeof(int), INT_BOX);
    fields = g_list_append(fields,databox1);
    DataBox *databox2 = databox_create(strdup("asf12"), 50, STRING_BOX);
    fields = g_list_append(fields,databox2);
    DataBox *databox3 = databox_create(strdup("asf12"), 60, STRING_BOX);
    fields = g_list_append(fields,databox3);

    heap_file_create(&allocator,"000","customer",fields);

    //open heap file
    int fd;
    HPErrorCode status = heap_file_open(&allocator,"000",&fd);
    if(status==HP_FILE_ERROR) printf("INCORRECT FILE\n");

    //Insert 100 records as a test
    for(int i=0;i<10;i++){
        Record *r = record_create();

        int *data = malloc(sizeof(int));
        *data = i;
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

    heap_file_print_all(&allocator,fd,fields,50+60+sizeof(int));

    heap_file_close(&allocator,fd);
    block_allocator_destroy(allocator);
    g_list_free_full(fields, databox_destroy);
    return 0;
}

