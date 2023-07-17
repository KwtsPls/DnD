#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "block.h"
#include "../memory/buffer_manager.h"

/******** DATABASE FILE HELPER FUNCTIONS *************/
//Function to initialize the functions of blocks/buffer
BlockAllocator *block_allocator_initialize(int size){
    BlockAllocator *allocator = malloc(sizeof(BlockAllocator));
    allocator->bufferManager = buffer_manager_create(size/BLOCK_SIZE);
    allocator->fds = malloc(MAX_OPEN_FILES*sizeof(int));
    allocator->files = malloc(MAX_OPEN_FILES * sizeof(char*));
    for(int i=0;i<MAX_OPEN_FILES;i++){
        allocator->fds[i]=0;
        allocator->files[i]=NULL;
    }
    return allocator;
}

//Function to destroy a block allocator
void block_allocator_destroy(BlockAllocator *allocator){
    for(int i=0;i<MAX_OPEN_FILES;i++){
        if(allocator->files[i]!=NULL)
            free(allocator->files[i]);
    }
    free(allocator->files);
    free(allocator->fds);
    buffer_manager_destroy(allocator->bufferManager);
    free(allocator);
}

//Get the name of an open file
char *block_file_name(BlockAllocator **allocator,int fd){
    for(int i=0;i<MAX_OPEN_FILES;i++){
        if((*allocator)->fds[i]==fd){
            return strdup((*allocator)->files[i]);
        }
    }
    return NULL;
}

//Function to create a new database file
int block_file_create(char *filename){
    int fd = creat(filename,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd!=-1){
        close(fd);
        return 1;
    }
    else return 0;
}

//Function to open a database file
int block_file_open(BlockAllocator **allocator,char *filename){
    int fd = open(filename,O_RDWR);
    for(int i=0;i<MAX_OPEN_FILES;i++){
        if((*allocator)->fds[i]==0){
            (*allocator)->fds[i]=fd;
            (*allocator)->files[i]=malloc(strlen(filename)+1);
            strcpy((*allocator)->files[i],filename);
            break;
        }
    }
    return fd;
}

//Function to close a database file
int block_file_close(BlockAllocator **allocator,int fd){
    for(int i=0;i<MAX_OPEN_FILES;i++){
        if((*allocator)->fds[i]==fd){
            (*allocator)->fds[i]=0;
            free((*allocator)->files[i]);
            (*allocator)->files[i]=NULL;
            break;
        }
    }
    return close(fd);
}

//Function to check the size of the file
int tell(int fd){
    return lseek(fd, 0, SEEK_END);
}

/********** BLOCK FUNCTIONS *****************/

//Function to allocate memory for a new block
Block *block_init(){
    Block *block = malloc(sizeof(Block));
    block->bit=0;
    block->block_number=-1;
    block->byteArray = NULL;
    return block;
}

//Function to allocate a new block at the end of a disk file
Block *block_allocate(BlockAllocator **allocator,int fd,Block *block){
    //File is empty
    if(tell(fd)==0){
        block->block_number=0;
        block->bit=0;
        block->byteArray = malloc(BLOCK_SIZE);
        memset(block->byteArray,0,BLOCK_SIZE);

        //Allocate disk space for a new block
        write(fd,block->byteArray,BLOCK_SIZE);
    }
    else{
        block->block_number = tell(fd)/BLOCK_SIZE;
        block->bit=0;
        block->byteArray = malloc(BLOCK_SIZE);
        memset(block->byteArray,0,BLOCK_SIZE);

        lseek(fd,0,SEEK_END);
        write(fd,block->byteArray,BLOCK_SIZE);
    }
    //Insert the new block into the buffer
    HashTableEntry *entry = hashtable_entry_create(block_file_name(allocator,fd),block->block_number);
    (*allocator)->bufferManager = buffer_manager_insert((*allocator)->bufferManager,entry,block->byteArray);

    return block;
}

//Function to write the contents of a block into the disk.
// Called only when the block is removed from memory
void block_write(int fd, Block *block){
    //set the fd to the location of the current block
    lseek(fd,BLOCK_SIZE*block->block_number,SEEK_SET);
    write(fd,block->byteArray,BLOCK_SIZE);
}

//Function to mark a block as dirty, so it can be written into disk
Block *block_set_dirty(Block *block){
    block->bit=1;
    return block;
}

//Function to remove a block from the buffer
void block_unpin(BlockAllocator **allocator,int fd, Block **block){
    if((*block)->bit==1){
        block_write(fd,*block);
        (*block)->bit=0;
    }
    //Set the current block as a candidate for removal
    (*allocator)->bufferManager = buffer_manager_set_inactive((*allocator)->bufferManager, block_file_name(allocator,fd),(*block)->block_number);
}

//Function to return a specific block
int block_get(BlockAllocator **allocator,int fd,int block_number,Block **block){
    if(block_number*BLOCK_SIZE > tell(fd))
        return -1;

    //Check if the block exists in the buffer - avoid seek if it exists
    char *byteArray = NULL;
    (*allocator)->bufferManager = buffer_manager_allocate((*allocator)->bufferManager,&byteArray, block_file_name(allocator,fd),block_number);

    //Block is not in memory - read from disk and insert the block to the buffer
    if(byteArray==NULL){
        lseek(fd,BLOCK_SIZE*block_number,SEEK_SET);
        (*block)->byteArray = malloc(BLOCK_SIZE);
        memset((*block)->byteArray,0,BLOCK_SIZE);
        read(fd,(*block)->byteArray,BLOCK_SIZE);
        HashTableEntry *entry = hashtable_entry_create(block_file_name(allocator,fd),block_number);
        (*allocator)->bufferManager = buffer_manager_insert((*allocator)->bufferManager,entry,(*block)->byteArray);
    }
    else{
        (*block)->byteArray = byteArray;
    }
    (*block)->bit=0;
    (*block)->block_number = block_number;

    return 1;
}

//Function to free the memory of the block struct
void block_destroy(Block *block){
    free(block);
}

