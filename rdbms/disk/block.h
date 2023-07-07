#ifndef DND_BLOCK_H
#define DND_BLOCK_H

#include "../memory/buffer_manager.h"

#define BLOCK_SIZE 512
#define FILE_SIZE 4096
#define MAX_OPEN_FILES 100

/* A struct implementing a block of fixed size loaded from the disk */
typedef struct block{
    int block_number;
    int bit;
    char *byteArray;
}Block;

typedef struct block_allocator{
    BufferManager *bufferManager;
    int *fds;
    char **files;
}BlockAllocator;

/******** DATABASE FILE HELPER FUNCTIONS *************/
//Function to initialize the functions of blocks/buffer
BlockAllocator * block_allocator_initialize(int size);
//Get the name of an open file
char *block_file_name(BlockAllocator **allocator,int fd);
//Function to create a new database file
int block_file_create(char *filename);
//Function to open a database file
int block_file_open(BlockAllocator **allocator,char *filename);
//Function to close a database file
int block_file_close(BlockAllocator **allocator,int fd);
//Function to check the size of the file
int tell(int fd);


/**************** BLOCK FUNCTIONS ********************/
//Function to allocate memory for a new block
Block *block_init();
//Function to allocate a new block at the end of a disk file
Block *block_allocate(BlockAllocator **allocator,int fd,Block *block);
//Function to write the contents of a block into the disk.
void block_write(int fd, Block *block);
//Function to mark a block as dirty, so it can be written into disk
Block *block_set_dirty(Block *block);
//Function to remove a block from the buffer
void block_unpin(BlockAllocator **allocator,int fd, Block **block);
//Function to return a specific block
int block_get(BlockAllocator **allocator,int fd,int block_number,Block **block);


#endif //DND_BLOCK_H
