#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <time.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./compiler/semantic.h"
#include "./disk/block.h"
#include "./db_files/heapfile.h"
#include "./db_files/db_file.h"

int main(int argc, char **argv){
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    Database *database = database_open("./data/medium/data0/");
    database = database_open_existing(database,"./data/medium/data1/");
    database = database_open_existing(database,"./data/medium/data2/");
    database = database_open_existing(database,"./data/medium/data3/");

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Loading time: %lf\n", cpu_time_used);

    // QUERY 1

    start = clock();
    GList *records = database_query(database,"SELECT COUNT(customer.id) FROM customer  WHERE customer.id>0");
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("QUERY 1 execution time: %lf\n", cpu_time_used);

    for(GList *node = records; node != NULL; node = node->next){
        Record *r = (Record*)node->data;
        record_print(r);
        printf("\n");
    }
    g_list_free_full(records, record_destroy);

    // QUERY 2

    start = clock();
    records = database_query(database,"SELECT COUNT(review.id) FROM customer,review WHERE customer.id = review.customer_id");
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("QUERY 2 execution time: %lf\n", cpu_time_used);

    for(GList *node = records; node != NULL; node = node->next){
        Record *r = (Record*)node->data;
        record_print(r);
        printf("\n");
    }
    g_list_free_full(records, record_destroy);

    // QUERY 3

    start = clock();
    records = database_query(database,"SELECT customer.id,customer.name,customer.surname FROM customer,review WHERE customer.id>20000 AND customer.id = review.customer_id ORDER BY customer.id ");
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("QUERY 3 execution time: %lf\n", cpu_time_used);

//    for(GList *node = records; node != NULL; node = node->next){
//        Record *r = (Record*)node->data;
//        record_print(r);
//        printf("\n");
//    }
    g_list_free_full(records, record_destroy);

    database_close(database);
}