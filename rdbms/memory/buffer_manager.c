#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "priority_queue.h"
#include "buffer_manager.h"

//Function to create a hashtable entry
HashTableEntry *hashtable_entry_create(char *filename,int id){
    HashTableEntry *entry = malloc(sizeof(HashTableEntry));
    entry->filename = filename;
    entry->id = id;
    return entry;
}

//Function to hash an entry
unsigned int entry_hash(const void *_entry){
    HashTableEntry *entry = (HashTableEntry*)_entry;
    char *str = malloc(strlen(entry->filename)+11);
    memset(str,0,strlen(entry->filename)+11);
    sprintf(str,"%s%d",entry->filename,entry->id);
    char *temp = str;

    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    free(temp);
    return hash;
}

//Function to compare two entries
int entry_equals(const void *a,const void *b){
    HashTableEntry *ea = (HashTableEntry*)a;
    HashTableEntry *eb = (HashTableEntry*)b;
    if(strcmp(ea->filename,eb->filename)==0 && ea->id == eb->id) return 1;
    else return 0;
}

//Function to create a hash table value
HashTableValue *hashtable_value_create(char **array,PQNode **node){
    HashTableValue *value = malloc(sizeof(HashTableValue));
    value->array = array;
    value->node = node;
    return value;
}

//Function to destroy a hash table value
void hashtable_value_destroy(void *_value){
    HashTableValue *value = (HashTableValue*)_value;
    free(*(value->array));
    free(value);
}

/********* BUFFER MANAGER FUNCTIONS *****************/

//Function to create a new buffer manager
BufferManager *buffer_manager_create(int size){
    BufferManager *bufferManager = malloc(sizeof(BufferManager));
    bufferManager->size = size;
    bufferManager->count=0;
    bufferManager->inactive_blocks=0;
    bufferManager->pq = pq_create();
    bufferManager->table = g_hash_table_new_full(entry_hash, entry_equals, NULL, hashtable_value_destroy);
    return bufferManager;
}

//Function to insert a new entry into the buffer manager
BufferManager *buffer_manager_insert(BufferManager *bufferManager,HashTableEntry *entry,char **array){
    //Create a new (key,value) pair to insert into the buffer
    PQNode *node = pq_node_create(entry);
    HashTableValue *value = hashtable_value_create(array,&node);

    g_hash_table_insert(bufferManager->table,entry,value);
    bufferManager->pq = pq_insert(bufferManager->pq,&node);

    return bufferManager;
}

//Function to set an entry to candidate for removal
BufferManager *buffer_manager_set_inactive(BufferManager *bufferManager,char *filename,int block_num){
    HashTableEntry *entry = hashtable_entry_create(filename,block_num);
    HashTableValue *value = (HashTableValue*)g_hash_table_lookup(bufferManager->table,entry);
    pq_node_set_inactive(value->node);
    return bufferManager;
}

//Function to return an entry if it exists in memory - or insert it
void buffer_manager_allocate(BufferManager *bufferManager,char **byteArray,char *filename,int block_num){
    HashTableEntry *entry = hashtable_entry_create(filename,block_num);
    HashTableValue *value = (HashTableValue*)g_hash_table_lookup(bufferManager->table,entry);

    //Block does not exist in the buffer
    if(value==NULL){
        *byteArray = NULL;
        return;
    }

    //Block exists in the buffer - update its priority and return its byteArray
    bufferManager->pq = pq_update(bufferManager->pq,*(value->node));
    byteArray = value->array;
    return;
}