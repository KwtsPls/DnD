#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "result_file.h"

//Function to create a result file
RFErrorCode result_file_create(BlockAllocator **allocator,char *filename, char *table1, char *field1,char *table2, char *field2){

    block_file_create(filename);
    int fd = block_file_open(allocator,filename);
    Block *block = block_init();

    //Create the first file
    block = block_allocate(allocator,fd,block);
    //filter descriptor
    int result_desc = 16;
    memcpy(block->byteArray,&result_desc, sizeof(int));
    block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Save the first table name
    block = block_allocate(allocator,fd,block);
    strcpy(block->byteArray,table1);
    block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Save the first field name
    block = block_allocate(allocator,fd,block);
    strcpy(block->byteArray,field1);
    block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Result occurred from join
    if(table2!=NULL){
        block_get(allocator,fd,0,&block);
        result_desc = 15;
        memcpy(block->byteArray,&result_desc, sizeof(int));
        block_set_dirty(block);
        block_unpin(allocator,fd,&block);

        //Save the first table name
        block = block_allocate(allocator,fd,block);
        strcpy(block->byteArray,table2);
        block_set_dirty(block);
        block_unpin(allocator,fd,&block);

        //Save the first field name
        block = block_allocate(allocator,fd,block);
        strcpy(block->byteArray,field2);
        block_set_dirty(block);
        block_unpin(allocator,fd,&block);
    }

    //Initialization is over - close file
    block_file_close(allocator,fd);
    block_destroy(block);

    return RF_OK;
}

//Function to open a result file
RFErrorCode result_file_open(BlockAllocator **allocator, char *filename, int *file_descriptor){
    Block *block = block_init();
    int fd = block_file_open(allocator,filename);

    //Check if current file is heap file
    block_get(allocator,fd,0,&block);
    int result_descriptor;
    memcpy(&result_descriptor,block->byteArray, sizeof(int));

    block_unpin(allocator,fd,&block);
    block_destroy(block);

    if(result_descriptor==15 || result_descriptor==16){
        *file_descriptor = fd;
        return RF_OK;
    }
    else{
        *file_descriptor=-1;
        block_file_close(allocator,fd);
        return RF_FILE_ERROR;
    }
}

//Function to close a heap file
RFErrorCode result_file_close(BlockAllocator **allocator,int fd){
    int status = block_file_close(allocator,fd);
    if(status==-1) return RF_CLOSE_ERROR;
    else return RF_OK;
}



