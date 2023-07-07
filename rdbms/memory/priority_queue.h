#ifndef DND_PRIORITY_QUEUE_H
#define DND_PRIORITY_QUEUE_H

#include "buffer_manager.h"

typedef struct hash_table_entry HashTableEntry;

typedef struct pq_node{
    HashTableEntry *entry;
    unsigned long priority;
    struct pq_node *next;
    struct pq_node *prev;
}PQNode;

typedef struct pq{
    PQNode *head;
    PQNode *tail;
}PriorityQueue;

//Function to create a pq node
PQNode *pq_node_create(HashTableEntry *entry,unsigned long priority);
//Function to delete a pq node
void pq_node_destroy(PQNode *pqNode);

//Function to create an empty pq
PriorityQueue *pq_create();
//Function to insert a new node in the priority queue
PriorityQueue *pq_insert(PriorityQueue *pq,PQNode *node);
//Function to insert a new node sorted based on priority
PriorityQueue *pq_sorted_insert(PriorityQueue *pq,HashTableEntry *entry,unsigned long priority);
//Function to pop the element with the lowest priority
PriorityQueue *pq_pop(PriorityQueue *pq);
//Function to remove a specific element from the priority queue
PriorityQueue *pq_remove(PriorityQueue *pq,HashTableEntry *entry);
//Function to destroy a priority queue
void pq_destroy(PriorityQueue *pq);
//Function to print the priority queue
void pq_print(PriorityQueue *pq);

#endif //DND_PRIORITY_QUEUE_H
