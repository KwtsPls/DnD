#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "priority_queue.h"

//Function to create a pq node
PQNode *pq_node_create(HashTableEntry *entry){
    PQNode *pqNode = malloc(sizeof(PQNode));
    pqNode->entry = entry;
    pqNode->active=1;
    pqNode->next=NULL;
    pqNode->prev=NULL;
    return pqNode;
}

//Function to delete a pq node
void pq_node_destroy(PQNode *pqNode){
    free(pqNode->entry->filename);
    free(pqNode->entry);
    free(pqNode);
}

//Function to change a node to inactive
void pq_node_set_inactive(PQNode **pqNode){
    (*pqNode)->active=0;
}

//Function to create an empty pq
PriorityQueue *pq_create(){
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->head=NULL;
    pq->tail=NULL;
    return pq;
}

//Function to insert a new node in the priority queue
PriorityQueue *pq_insert(PriorityQueue *pq,PQNode **node){

    //Priority queue is empty
    if(pq->head==NULL && pq->tail==NULL){
        pq->head = *node;
        pq->tail = *node;
    }
    else{
        PQNode *prev = pq->tail;
        pq->tail->next = *node;
        pq->tail = *node;
        (*node)->prev = prev;
    }

    return pq;
}

//Function to pop the element with the lowest priority
//Only inactive entries are eligible for removal
PriorityQueue *pq_pop(PriorityQueue *pq){
    if(pq==NULL || pq->head==NULL) return NULL;

    PQNode *cur = pq->head;
    while(cur!=NULL){
        if(cur->active==0){
            //List contains only one node
            if(pq->head == cur && pq->tail == cur){
                pq->head = pq->tail = NULL;
            }
            else if(pq->head == cur && pq->tail != cur){
                pq->head = cur->next;
                cur->next->prev=NULL;
            }
            else if(pq->head != cur && pq->tail == cur){
                pq->tail = cur->prev;
                cur->prev->next=NULL;
            }
            else{
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
            }
            pq_node_destroy(cur);
            return pq;
        }
        cur = cur->next;
    }

    return pq;
}

//Function to move a node to the end of the queue
PriorityQueue *pq_update(PriorityQueue *pq,PQNode *node){
    if(pq==NULL || pq->head==NULL) return NULL;

    //List contains only one node
    if(pq->head == node && pq->tail == node){
        return pq;
    }
    else if(pq->head == node && pq->tail != node){
        pq->head = node->next;
        node->next->prev=NULL;
    }
    else if(pq->head != node && pq->tail == node){
        return pq;
    }
    else{
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    //Current node is most recently used - push to the end of list
    PQNode *prev = pq->tail;
    node->next=NULL;
    node->prev=NULL;
    pq->tail->next = node;
    pq->tail = node;
    node->prev = prev;

    return pq;
}

//Function to print the priority queue
void pq_print(PriorityQueue *pq){
    PQNode *cur = pq->head;
    while(cur!=NULL){
        printf("(%s-%d)",cur->entry->filename,cur->entry->id);
        cur = cur->next;
    }
    printf("\n");
}
