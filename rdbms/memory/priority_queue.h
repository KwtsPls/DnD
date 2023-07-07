#ifndef DND_PRIORITY_QUEUE_H
#define DND_PRIORITY_QUEUE_H

#include "buffer_manager.h"

typedef struct hash_table_entry HashTableEntry;

typedef struct pq_node{
    HashTableEntry *entry;
    int active;
    struct pq_node *next;
    struct pq_node *prev;
}PQNode;

typedef struct pq{
    PQNode *head;
    PQNode *tail;
}PriorityQueue;

//Function to create a pq node
PQNode *pq_node_create(HashTableEntry *entry);
//Function to delete a pq node
void pq_node_destroy(PQNode *pqNode);
//Function to change a node to inactive
void pq_node_set_inactive(PQNode **pqNode);

//Function to create an empty pq
PriorityQueue *pq_create();
//Function to insert a new node in the priority queue
PriorityQueue *pq_insert(PriorityQueue *pq,PQNode **node);
//Function to pop the element with the lowest priority
PriorityQueue *pq_pop(PriorityQueue *pq);
//Function to print the priority queue
void pq_print(PriorityQueue *pq);
//Function to move a node to the end of the queue
PriorityQueue *pq_update(PriorityQueue *pq,PQNode *node);

#endif //DND_PRIORITY_QUEUE_H
