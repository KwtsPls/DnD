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
    if(strcmp(ea->filename,eb->filename)==0 && (ea->id == eb->id)) return TRUE;
    else return FALSE;
}

//Function to destroy a hash table entry
void hashtable_entry_destroy(void *_entry){
    HashTableEntry *entry = (HashTableEntry*)_entry;
    free(entry->filename);
    free(entry);
}

//Function to create a hash table value
HashTableValue *hashtable_value_create(char *array,unsigned long priority){
    HashTableValue *value = malloc(sizeof(HashTableValue));
    value->array = array;
    value->priority = priority;
    value->active=1;
    return value;
}

//Function to destroy a hash table value
void hashtable_value_destroy(void *_value){
    HashTableValue *value = (HashTableValue*)_value;
    if(value->array!=NULL) {
        free(value->array);
        value->array = NULL;
    }
    free(value);
}

/********* BUFFER MANAGER FUNCTIONS *****************/

//Function to create a new buffer manager
BufferManager *buffer_manager_create(int size){
    BufferManager *bufferManager = malloc(sizeof(BufferManager));
    bufferManager->size = size;
    bufferManager->count=0;
    bufferManager->priority_counter=0;
    bufferManager->pq = pq_create();
    bufferManager->table = g_hash_table_new_full(entry_hash, entry_equals, hashtable_entry_destroy, hashtable_value_destroy);
    return bufferManager;
}

//Function to insert a new entry into the buffer manager
BufferManager *buffer_manager_insert(BufferManager *bufferManager,HashTableEntry *entry,char *array){
    //There is no more space in the buffer - pop a candidate
    if(bufferManager->count == bufferManager->size){
        if(bufferManager->pq->head!=NULL){
            //Remove the entry from the buffer hash table
            g_hash_table_remove(bufferManager->table,bufferManager->pq->head->entry);

            //Pop the least recently used element
            bufferManager->pq = pq_pop(bufferManager->pq);
            bufferManager->count--;
        }
    }

    //Create a new (key,value) pair to insert into the buffer
    HashTableValue *value = hashtable_value_create(array,bufferManager->priority_counter);
    bufferManager->priority_counter++;
    bufferManager->count++;

    g_hash_table_insert(bufferManager->table,entry,value);

    return bufferManager;
}

//Function to set an entry to candidate for removal
BufferManager *buffer_manager_set_inactive(BufferManager *bufferManager,char *filename,int block_num){
    HashTableEntry *entry = hashtable_entry_create(filename,block_num);
    HashTableValue *value = (HashTableValue*)g_hash_table_lookup(bufferManager->table,entry);

    //Current block is set as candidate for removal
    value->active=0;
    bufferManager->pq = pq_insert(bufferManager->pq, pq_node_create(entry,bufferManager->priority_counter));
    bufferManager->count--;

    return bufferManager;
}

//Function to return an entry if it exists in memory - or insert it
BufferManager *buffer_manager_allocate(BufferManager *bufferManager,char **byteArray,char *filename,int block_num){
    HashTableEntry *entry = hashtable_entry_create(filename,block_num);
    HashTableValue *value = (HashTableValue*)g_hash_table_lookup(bufferManager->table,entry);

    //Block does not exist in the buffer
    if(value==NULL){
        hashtable_entry_destroy(entry);
        *byteArray = NULL;
        return bufferManager;
    }

    //Block exists in the buffer - update its priority and return its byteArray
    if(value->active==0){
        bufferManager->pq = pq_remove(bufferManager->pq, entry);
        value->active=1;
        bufferManager->count++;
    }
    //renew the priority of the current block
    value->priority = bufferManager->priority_counter;
    bufferManager->priority_counter++;

    //get the data from memory
    *byteArray = value->array;

    hashtable_entry_destroy(entry);
    return bufferManager;
}

//Function to return the available space in a buffer manager
int buffer_manager_available_space(BufferManager *bufferManager){
    return bufferManager->size - bufferManager->count;
}

//Function to destroy the buffer manager
void buffer_manager_destroy(BufferManager *bufferManager){
    g_hash_table_destroy(bufferManager->table);
    pq_destroy(bufferManager->pq);
    free(bufferManager);
}