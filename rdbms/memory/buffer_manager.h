#ifndef DND_BUFFER_MANAGER_H
#define DND_BUFFER_MANAGER_H

#include <glib.h>
#include "priority_queue.h"

#define BUFFER_SIZE 51200

typedef struct pq_node PQNode;
typedef struct pq PriorityQueue;

typedef struct buffer_manager{
    int size;
    int count;
    int inactive_blocks;
    GHashTable *table;
    PriorityQueue *pq;
}BufferManager;

typedef struct hash_table_entry{
    char *filename;
    int id;
}HashTableEntry;

typedef struct hash_table_value{
    char **array;
    PQNode **node;
}HashTableValue;

//Function to create a hashtable entry
HashTableEntry *hashtable_entry_create(char *filename,int id);
//Function to hash an entry
unsigned int entry_hash(const void *_entry);
//Function to compare two entries
int entry_equals(const void *a,const void *b);
//Function to create a hash table value
HashTableValue *hashtable_value_create(char **array,PQNode **node);
//Function to destroy a hash table value
void hashtable_value_destroy(void *_value);

//Function to create a new buffer manager
BufferManager *buffer_manager_create(int size);
//Function to set an entry to candidate for removal
BufferManager *buffer_manager_set_inactive(BufferManager *bufferManager,char *filename,int block_num);
//Function to insert a new entry into the buffer manager
BufferManager *buffer_manager_insert(BufferManager *bufferManager,HashTableEntry *entry,char **array);
//Function to return an entry if it exists in memory - or insert it
void buffer_manager_allocate(BufferManager *bufferManager,char **byteArray,char *filename,int block_num);

#endif //DND_BUFFER_MANAGER_H
