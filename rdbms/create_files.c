#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "./disk/block.h"
#include "./table/databox.h"
#include "./table/record.h"
#include "./db_files/heapfile.h"

BlockAllocator* create_heapfile(gchar *table_name,
                                GList *fields,
                                GList *field_names,
                                gchar *filename){
    BlockAllocator *allocator = block_allocator_initialize(BUFFER_SIZE*1024);
    heap_file_create(&allocator,filename,table_name,fields,field_names);
    return allocator;
}

#define ADD_FIELD(FIELDS_LIST, FIELD_NAMES_LIST, NAME, TYPE, SIZE) \
    if ((TYPE) == INT_BOX) {        \
        int *i = malloc(sizeof(int)); \
        *i = 125; \
        FIELDS_LIST = g_list_append(FIELDS_LIST, databox_create(i, (SIZE), (TYPE))); \
    }                               \
    if ((TYPE) == DOUBLE_BOX) {        \
        double *i = malloc(sizeof(double)); \
        *i = 125.5; \
        FIELDS_LIST = g_list_append(FIELDS_LIST, databox_create(i, (SIZE), (TYPE))); \
    }                               \
    if ((TYPE) == STRING_BOX) {        \
        FIELDS_LIST = g_list_append(FIELDS_LIST, databox_create(strdup("asf"), (SIZE), (TYPE))); \
    }                                \
    FIELD_NAMES_LIST = g_list_append(FIELD_NAMES_LIST, strdup((NAME)));

#define RECORD_ADD_FIELD_INT(RECORD, VALUE) \
    {                                                 \
        int *data = malloc(sizeof(int)); \
        *data=VALUE; \
        DataBox *databox = databox_create(data, sizeof(int),INT_BOX); \
        r = record_add_field(r,databox); \
    }

#define RECORD_ADD_FIELD_STRING(RECORD, STRING, SIZE) \
    {                                                 \
        char *data_str = malloc(SIZE); \
        memset(data_str,0,SIZE); \
        strcpy(data_str,STRING); \
        DataBox *databox = databox_create(data_str,SIZE,STRING_BOX); \
        RECORD = record_add_field(RECORD,databox);      \
    }


const char* NAMES_FILEPATH = "./names.txt";

GArray* load_names() {
    GArray *names = g_array_new (FALSE, TRUE, sizeof (char*));

    FILE * fp;
    char * line = NULL;
    size_t len = 0;

    fp = fopen(NAMES_FILEPATH, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while (getline(&line, &len, fp) != -1) {
        g_strchomp (line);
        printf("Read line: %s\n", line);
        gchar *copy = g_strdup (line);
        g_array_append_val (names, copy);
    }

    fclose(fp);

    return names;
}

const unsigned CUSTOMER_COUNT = 10000;
const unsigned PURCHASE_COUNT = 50000;
const unsigned REVIEW_PROBABILITY = 4; // p = 1 / REVIEW_PROBABILITY

int create_customer(){
    BlockAllocator *allocator = NULL;
    Table *table=NULL;

    GList *fields = NULL;
    GList *field_names = NULL;

    ADD_FIELD(fields, field_names, "id", INT_BOX, sizeof(int))
    ADD_FIELD(fields, field_names, "name", STRING_BOX, 50)
    ADD_FIELD(fields, field_names, "surname", STRING_BOX, 60)

    allocator = create_heapfile("customer", fields, field_names, "data/customer");

    GArray *names = load_names();

    //open heap file
    int fd;
    HPErrorCode status = heap_file_open(&allocator,"data/customer",&fd,&table);
    if(status==HP_FILE_ERROR) g_error("INCORRECT FILE\n");

    //Insert 100 records as a test
    for(int i=0;i<CUSTOMER_COUNT;i++){
        Record *r = record_create();
        RECORD_ADD_FIELD_INT(r, i)
        RECORD_ADD_FIELD_STRING(r, g_array_index(names, char*, rand() % names->len), 50)
        RECORD_ADD_FIELD_STRING(r, g_array_index(names, char*, rand() % names->len), 60)

        heap_file_insert(&allocator,r,fd);
        record_destroy(r);
    }

    heap_file_print_all(&allocator,fd,fields,50+60+sizeof(int));

    heap_file_close(&allocator,fd,&table);
    block_allocator_destroy(allocator);
    g_list_free_full(fields, databox_destroy);
    g_list_free_full(field_names, free);

    return 0;
}

int create_purchase_and_review(){
    //
    // purchase
    //
    BlockAllocator *p_allocator = NULL;
    Table *p_table=NULL;

    GList *p_fields = NULL;
    GList *p_field_names = NULL;

    ADD_FIELD(p_fields, p_field_names, "id", INT_BOX, sizeof(int));
    ADD_FIELD(p_fields, p_field_names, "customer_id", INT_BOX, sizeof(int));
    ADD_FIELD(p_fields, p_field_names, "description", STRING_BOX, 50);

    p_allocator = create_heapfile("purchase", p_fields, p_field_names, "data/purchase");
    //
    // review
    //
    BlockAllocator *r_allocator = NULL;
    Table *r_table=NULL;

    GList *r_fields = NULL;
    GList *r_field_names = NULL;

    ADD_FIELD(r_fields, r_field_names, "id", INT_BOX, sizeof(int));
    ADD_FIELD(r_fields, r_field_names, "customer_id", INT_BOX, sizeof(int));
    ADD_FIELD(r_fields, r_field_names, "purchase_id", INT_BOX, sizeof(int));
    ADD_FIELD(r_fields, r_field_names, "review", STRING_BOX, 300);

    r_allocator = create_heapfile("review", r_fields, r_field_names, "data/review");
    //
    // purchase
    //
    int p_fd;
    HPErrorCode status = heap_file_open(&p_allocator, "data/purchase", &p_fd, &p_table);
    if(status==HP_FILE_ERROR) g_error("INCORRECT FILE\n");
    //
    // review
    //
    int r_fd;
    status = heap_file_open(&r_allocator, "data/review", &r_fd, &r_table);
    if(status==HP_FILE_ERROR) g_error("INCORRECT FILE\n");
    //
    // insert records (purchase & review)
    //
    int review_count = 0;
    for(int i=0;i<PURCHASE_COUNT;i++){
        Record *r = record_create();

        int customed_id = rand() % CUSTOMER_COUNT;
        RECORD_ADD_FIELD_INT(r, i)
        RECORD_ADD_FIELD_INT(r, customed_id)
        RECORD_ADD_FIELD_STRING(r, "Purchase Description", 50)

        heap_file_insert(&p_allocator, r, p_fd);
        record_destroy(r);

        // add a review for this purchase
        if ((rand() % REVIEW_PROBABILITY) == 0) {
            Record *r = record_create();

            RECORD_ADD_FIELD_INT(r, review_count)
            RECORD_ADD_FIELD_INT(r, customed_id)
            RECORD_ADD_FIELD_INT(r, i)
            RECORD_ADD_FIELD_STRING(r, "Placeholder Review", 300)

            heap_file_insert(&r_allocator, r, r_fd);
            record_destroy(r);

            review_count++;
        }
    }
    //
    // purchase
    //
    heap_file_print_all(&p_allocator, p_fd, p_fields, 50 + sizeof(int) + sizeof(int));

    heap_file_close(&p_allocator, p_fd, &p_table);
    block_allocator_destroy(p_allocator);
    g_list_free_full(p_fields, databox_destroy);
    g_list_free_full(p_field_names, free);
    //
    // review
    //
    heap_file_print_all(&r_allocator,r_fd,r_fields,300+sizeof(int)+sizeof(int)+sizeof(int));

    heap_file_close(&r_allocator,r_fd,&r_table);
    block_allocator_destroy(r_allocator);
    g_list_free_full(r_fields, databox_destroy);
    g_list_free_full(r_field_names, free);

    return 0;
}

int main() {
    srand(1);

    create_customer();
    create_purchase_and_review ();

    return 0;
}
