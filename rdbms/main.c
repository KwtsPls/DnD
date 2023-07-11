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
#include "./joins/bnl.h"

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
        *data = rand()%100;
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

    //Create second heap file
    Table *table2=NULL;

    GList *fields2 = NULL;
    GList *field_names2 = NULL;
    int *i2 = malloc(sizeof(int));
    *i2 = 125;
    DataBox *databox = databox_create(i2, sizeof(int), INT_BOX);
    fields2 = g_list_append(fields2,databox);
    databox = databox_create(strdup("asf12"), 50, STRING_BOX);
    fields2 = g_list_append(fields2,databox);


    f = strdup("id");
    field_names2 = g_list_append(field_names2,f);
    f = strdup("fielder");
    field_names2 = g_list_append(field_names2,f);

    heap_file_create(&allocator,"111","orders",fields2,field_names2);

    //open heap file
    int fd2;
    status = heap_file_open(&allocator,"111",&fd2,&table2);
    if(status==HP_FILE_ERROR) printf("INCORRECT FILE\n");

    //Insert 100 records as a test
    for(int i=0;i<50;i++){
        Record *r = record_create();

        int *data = malloc(sizeof(int));
        *data = rand()%100;
        DataBox *databox = databox_create(data, sizeof(int),INT_BOX);
        r = record_add_field(r,databox);

        char *data_str = malloc(50);
        memset(data_str,0,50);
        strcpy(data_str,"A FIELD RECORD");
        databox = databox_create(data_str,50,STRING_BOX);
        r = record_add_field(r,databox);

        heap_file_insert(&allocator,r,fd2);
        record_destroy(r);
    }

    ResultSet *set1 = heap_file_bnl(&allocator,&table,&table2,"id","id",EQUAL);
    /*int d=50;
    ResultSet *set2 = heap_file_filter(&allocator,&table,"id",GREATER,&d);
    ResultSet *set = result_set_and(set1,set2);
    for (GList *lp = set->results; lp != NULL; lp = lp->next) {
        ResultItem *item = lp->data;
        for (GList *lpp = item->records; lpp != NULL; lpp = lpp->next) {
            Record *record = lpp->data;
            record_print (record);
        }
        printf("\n");
    }*/

    GList *tokens = tokenize("SELECT customer.id,orders.fielder FROM customer,orders WHERE customer.id=orders.id "
                             "ORDER BY customer.id LIMIT 1");
    Statement *statement = parse_statement(&tokens);

    DBFile *dbFile1 = malloc(sizeof(DBFile));
    dbFile1->table = table;
    dbFile1->allocator = allocator;
    dbFile1->fd = fd;

    DBFile *dbFile2 = malloc(sizeof(DBFile));
    dbFile2->table = table2;
    dbFile2->allocator = allocator;
    dbFile2->fd = fd2;

    GList *db_file_list = NULL;
    db_file_list = g_list_append(db_file_list,dbFile1);
    db_file_list = g_list_append(db_file_list,dbFile2);


    GList *records = result_set_finalize(set1,statement,db_file_list);

    for (GList *lp = records; lp != NULL; lp = lp->next){
        Record *r = (Record*)lp->data;
        record_print(r);
        printf("\n");
    }


    heap_file_close(&allocator,fd,&table);
    heap_file_close(&allocator,fd2,&table2);
    block_allocator_destroy(allocator);
    g_list_free_full(records, record_destroy);
    g_list_free_full(fields, databox_destroy);
    g_list_free_full(field_names, free);
    g_list_free_full(fields2, databox_destroy);
    g_list_free_full(field_names2, free);
    return 0;
}

