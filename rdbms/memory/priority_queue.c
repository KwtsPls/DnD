#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "priority_queue.h"

//Function to create a pq node
PQNode *pq_node_create(HashTableEntry *entry,unsigned long priority){
    PQNode *pqNode = malloc(sizeof(PQNode));
    pqNode->entry = entry;
    pqNode->priority = priority;
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

//Function to create an empty pq
PriorityQueue *pq_create(){
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->head=NULL;
    pq->tail=NULL;
    return pq;
}

//Function to insert a new node in the priority queue
PriorityQueue *pq_insert(PriorityQueue *pq,PQNode *node){

    //Priority queue is empty
    if(pq->head==NULL && pq->tail==NULL){
        pq->head = node;
        pq->tail = node;
    }
    else{
        PQNode *prev = pq->tail;
        pq->tail->next = node;
        pq->tail = node;
        node->prev = prev;
    }

    return pq;
}

//Function to insert a new node sorted based on priority
PriorityQueue *pq_sorted_insert(PriorityQueue *pq,HashTableEntry *entry,unsigned long priority){
    if(pq==NULL) return NULL;
    PQNode *node = pq_node_create(entry,priority);

    //Priority queue is empty
    if(pq->head==NULL && pq->tail==NULL){
        pq->head = node;
        pq->tail = node;
    }
    else{
        if(pq->head->priority > node->priority){
            node->next = pq->head;
            pq->head->prev = node;
            pq->head = node;
        }
        else if(pq->tail->priority < node->priority){
            node->prev = pq->tail;
            pq->tail->next = node;
            pq->tail = node;
        }
        else{
            PQNode *cur = pq->head;
            while(cur!=NULL){
                if(cur->priority < node->priority){
                    node->prev = cur;
                    node->next = cur->next;
                    cur->next->prev = node;
                    cur->next = node;
                    return pq;
                }
                cur = cur->next;
            }
        }
    }

    return pq;
}

//Function to pop the element with the smallest priority
PriorityQueue *pq_pop(PriorityQueue *pq){
    if(pq==NULL || pq->head) return pq;

    PQNode *del = pq->head;
    if(pq->head==pq->tail){
        pq->head=NULL;
        pq->tail=NULL;
    }
    else{
        pq->head->next->prev=NULL;
        pq->head = pq->head->next;
    }

    pq_node_destroy(del);
    return pq;
}

//Function to remove a specific element from the priority queue
PriorityQueue *pq_remove(PriorityQueue *pq,HashTableEntry *entry){
    if(pq==NULL || pq->head==NULL) return NULL;

    PQNode *cur = pq->head;
    while(cur!=NULL){
        if(entry_equals(cur->entry,entry)==1){
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

//Function to destroy a priority queue
void pq_destroy(PriorityQueue *pq){
    PQNode *cur = pq->head;
    PQNode *temp = pq->head;
    while(cur != NULL){
        temp = cur->next;
        pq_node_destroy(cur);
        cur = temp;
    }
    free(pq);
}

//Function to print the priority queue
void pq_print(PriorityQueue *pq){
    PQNode *cur = pq->head;
    while(cur!=NULL){
        printf("(%s-%d,%ld)",cur->entry->filename,cur->entry->id,cur->priority);
        cur = cur->next;
    }
    printf("\n");
}


