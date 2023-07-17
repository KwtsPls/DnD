#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "hash_join.h"

typedef struct hash_join_key{
    int i;
    Record *r;
}HashJoinKey;

unsigned int hash_join_h(const void *data){
    HashJoinKey *key = (HashJoinKey*)data;
    DataBox *databox = record_get_field_value(key->r,key->i);
    char *str=NULL;
    if(databox->type==INT_BOX){
        str=malloc(10);
        memset(str,0,10);
        sprintf(str,"%d",*(int*)databox->data);
    }
    else if(databox->type==DOUBLE_BOX){
        str=malloc(20);
        memset(str,0,20);
        sprintf(str,"%f",*(double*)databox->data);
    }
    else{
        str = strdup((char*)databox->data);
    }

    char *temp = str;

    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    free(temp);
    return hash;
}

int hash_join_equals(const void *_a,const void *_b){
    HashJoinKey *key1 = (HashJoinKey*)_a;
    HashJoinKey *key2 = (HashJoinKey*)_b;

    DataBox *databox1 = record_get_field_value(key1->r,key1->i);
    DataBox *databox2 = record_get_field_value(key2->r,key2->i);

    if(databox_compare(databox1,databox2)==0) return TRUE;
    else return FALSE;
}

int hash_join_unequals(const void *_a,const void *_b){
    HashJoinKey *key1 = (HashJoinKey*)_a;
    HashJoinKey *key2 = (HashJoinKey*)_b;

    DataBox *databox1 = record_get_field_value(key1->r,key1->i);
    DataBox *databox2 = record_get_field_value(key2->r,key2->i);

    if(databox_compare(databox1,databox2)!=0) return TRUE;
    else return FALSE;
}

void hash_join_key_destroy(void *a){
    HashJoinKey *key = a;
    record_destroy(key->r);
    free(key);
}

void hash_join_value_destroy_nothing(void *a){
    return;
}

void hash_join_value_destroy(void *a){
    GList *list = a;
    g_list_free_full(list,hash_join_value_destroy_nothing);
}

void hash_join_not_equal_value_destroy(void *a){
    GList *list = a;
    g_list_free_full(list,record_destroy);
}

/******************** HASH JOIN IMPLEMENTATION ************************/

//Function to perform a hash join on heap files
ResultSet *heap_file_hash_join(BlockAllocator **allocator,Table **table1,Table **table2,char *field1,char *field2,TokenType op){
    int field_pos1= table_field_pos(*table1,field1);
    int field_pos2= table_field_pos(*table2,field2);

    int fd1 = (*table1)->fd;
    int fd2 = (*table2)->fd;

    GList *f1 = (*table1)->fields;
    GList *f2 = (*table2)->fields;

    int left_block_num = tell(fd1)/BLOCK_SIZE;
    int right_block_num = tell(fd2)/BLOCK_SIZE;

    int size1 = (*table1)->record_size;
    int size2 = (*table2)->record_size;

    //Create a new result set for the join
    ResultSet *set = result_set_create();

    //Inner table must have fewer blocks
    if(right_block_num>left_block_num){
        int t = fd1;
        fd1 = fd2;
        fd2 = t;

        t = left_block_num;
        left_block_num = right_block_num;
        right_block_num = t;

        t = size1;
        size1 = size2;
        size2 = t;

        t = field_pos1;
        field_pos1 = field_pos2;
        field_pos2 = t;

        GList *lt = f1;
        f1 = f2;
        f2 = lt;

        set = result_set_add_table(set,*table2);
        set = result_set_add_table(set,*table1);
    }
    else{
        set = result_set_add_table(set,*table1);
        set = result_set_add_table(set,*table2);
    }

    Block *block = block_init();

    //Create a hash table from the smaller table
    GHashTable *hash_table = NULL;
    if(op==EQUAL)
        hash_table = g_hash_table_new_full(hash_join_h, hash_join_equals,hash_join_key_destroy,hash_join_value_destroy);
    else
        hash_table = g_hash_table_new_full(hash_join_h, hash_join_equals,hash_join_key_destroy,
                                           hash_join_not_equal_value_destroy);

    if(op==EQUAL) {
        for (int i = 3; i < right_block_num; i++) {
            block_get(allocator, fd2, i, &block);
            char offset = block->byteArray[0];
            for (int j = 0; j < offset; j++) {
                Record *r = heap_file_map_record(block, f2, j, size2);
                HashJoinKey *key = malloc(sizeof(HashJoinKey));
                key->r = r;
                key->i = field_pos2;

                GList *value = g_hash_table_lookup(hash_table, key);

                GList *value_list = NULL;
                if (value == NULL) {
                    value_list = g_list_append(value_list, key->r);
                } else {
                    value_list = g_list_copy(value);
                    value_list = g_list_append(value_list, key->r);
                }
                g_hash_table_insert(hash_table, key, value_list);
            }
            block_unpin(allocator, fd2, &block);
        }
    }
    else{
        Block *block2 = block_init();
        for(int i=3;i<right_block_num;i++){
            block_get(allocator, fd2, i, &block);
            int offset1 = block->byteArray[0];
            for(int t1=0;t1<offset1;t1++){
                Record *tr = heap_file_map_record(block,f2,t1,size2);
                GList *not_equal_list=NULL;

                for(int j=3;j<right_block_num;j++){
                    block_get(allocator, fd2, j, &block2);
                    int offset2 = block2->byteArray[0];
                    for(int t2=0;t2<offset2;t2++){
                        Record *ts = heap_file_map_record(block2,f2,t2,size2);
                        if(databox_compare(record_get_field_value(tr,field_pos2),record_get_field_value(ts,field_pos2))!=0)
                            not_equal_list = g_list_append(not_equal_list,ts);
                        else
                            record_destroy(ts);
                    }
                }

                HashJoinKey *key = malloc(sizeof(HashJoinKey));
                key->r = tr;
                key->i = field_pos2;
                g_hash_table_insert(hash_table, key, not_equal_list);
            }
        }

        for(int i=3;i<right_block_num;i++){
            block2->block_number=i;
            block_unpin(allocator,fd2,&block2);
        }

        block_destroy(block2);
    }

    //Match the records from the second table in the in-memory hashtable
    for(int i=3;i<left_block_num;i++){
        block_get(allocator,fd1,i,&block);
        char offset = block->byteArray[0];
        for(int j=0;j<offset;j++){
            Record *tr = heap_file_map_record(block,f1,j,size1);

            HashJoinKey *key = malloc(sizeof(HashJoinKey));
            key->r = tr;
            key->i = field_pos1;
            GList *value = g_hash_table_lookup(hash_table,key);

            if(value!=NULL){
                for(GList *node = value; node != NULL; node = node->next){
                    Record *ts = node->data;
                    ResultItem *item = result_item_create();
                    item = result_item_add_record(item,record_copy(tr,NULL));
                    item = result_item_add_record(item, record_copy(ts,NULL));
                    set = result_set_add_item(set,item);
                }
            }
            hash_join_key_destroy(key);
        }
        block_unpin(allocator,fd1,&block);
    }

    block_destroy(block);
    g_hash_table_destroy(hash_table);

    return set;
}

