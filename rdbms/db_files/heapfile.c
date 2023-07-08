#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <glib.h>
#include "heapfile.h"

//Function to create a heap file as a database file
HPErrorCode heap_file_create(BlockAllocator **allocator, char *filename, char *table_name, GList *fields){

    //Create a block file
    block_file_create(filename);
    int fd = block_file_open(allocator,filename);

    //Initialize a block structure to begin disk operations
    Block *block = block_init();

    block = block_allocate(allocator,fd,block);
    //At the start of the first block - write heap file descriptor
    int heap_descriptor = 17;
    memcpy(block->byteArray,&heap_descriptor,sizeof(int));

    int fields_num = g_list_length(fields);
    memcpy(block->byteArray + sizeof(int), &fields_num, sizeof(int));

    unsigned long long records_num = 0;
    memcpy(block->byteArray + sizeof(int)*2,&records_num, sizeof(unsigned long long));

    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Second block of the file holds only the name of the table
    block = block_allocate(allocator,fd,block);
    strcpy(block->byteArray,table_name);

    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Third block contains (field_type,field_size_ pairs
    block = block_allocate(allocator,fd,block);
    for(int i=0;i<fields_num;i++){
        DataBox *databox = (DataBox*)g_list_nth(fields,i)->data;
        int type = databox->type;
        int size = databox->size;


        memcpy(block->byteArray + sizeof(int)*i,&type, sizeof(int));
        memcpy(block->byteArray + sizeof(int) + sizeof(int)*i,&size, sizeof(int));
    }
    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Initialization is over - close file
    block_file_close(allocator,fd);
    block_destroy(block);

    return HP_OK;
}

//Function to open a heap file
HPErrorCode heap_file_open(BlockAllocator **allocator,char *filename,int *file_descriptor){
    Block *block = block_init();
    int fd = block_file_open(allocator,filename);

    //Check if current file is heap file
    block_get(allocator,fd,0,&block);
    int heap_descriptor;
    memcpy(&heap_descriptor,block->byteArray, sizeof(int));

    block_unpin(allocator,fd,&block);
    block_destroy(block);

    if(heap_descriptor==17){
        *file_descriptor = fd;
        return HP_OK;
    }
    else{
        *file_descriptor=-1;
        heap_file_close(allocator,fd);
        return HP_FILE_ERROR;
    }
}

//Function to insert a record in the heap file
HPErrorCode heap_file_insert(BlockAllocator **allocator,Record *record,int fd){
    //Get the number of blocks in the file
    int blocks_num = tell(fd)/BLOCK_SIZE;
    Block *block=block_init();

    //Heap file is not empty - check if last block has enough space
    int empty=1;
    if(blocks_num>3){
        block_get(allocator,fd,blocks_num-1,&block);
        //check the record counter in the block
        if(block->byteArray[0] < (BLOCK_SIZE/record->size)){
            int offset = block->byteArray[0];
            heap_file_write_record(block,record,offset);
            offset++;
            memset(block->byteArray,offset, sizeof(char));
            empty=0;
            block_set_dirty(block);
        }
        block_unpin(allocator,fd,&block);
    }

    //Heap file is either empty or last block is full - allocate new block
    if(empty==1){
        block = block_allocate(allocator,fd,block);
        //set the record counter to 1
        memset(block->byteArray,1, sizeof(char));
        heap_file_write_record(block,record,0);
        block_set_dirty(block);
        block_unpin(allocator,fd,&block);
    }

    block_destroy(block);
    return HP_OK;
}

//Function to close a heap file
HPErrorCode heap_file_close(BlockAllocator **allocator,int fd){
    int status = block_file_close(allocator,fd);
    if(status==-1) return HP_CLOSE_ERROR;
    else return HP_OK;
}

/************ HELPER FUNCTIONS ******************/
//Function to write a record inside a block
void heap_file_write_record(Block *block,Record *r,int offset){
    int n = g_list_length(r->list);
    int sum = 0;
    for(int i=0;i<n;i++){
        DataBox *databox = (DataBox*)g_list_nth(r->list,i)->data;
        memcpy(block->byteArray + sizeof(char) + r->size * offset + sum,databox->data, databox->size );
        sum += databox->size;
    }
}

//Function to map fields to a record
Record *heap_file_map_record(Block *block,GList *fields,int offset,int size){
    Record *record = record_create();

    int sum=0;
    int n = g_list_length(fields);
    for(int i=0;i<n;i++){
        DataBox *field = (DataBox*)g_list_nth(fields,i)->data;

        void *data =malloc(field->size);
        memcpy(data, block->byteArray + sizeof(char) + offset * size + sum, field->size);
        DataBox *databox = databox_create(data,field->size,field->type);
        record = record_add_field(record,databox);
        sum += field->size;
    }

    return record;
}

//Function to print all records in the file
void heap_file_print_all(BlockAllocator **allocator,int fd,GList *fields,int size){
    Block *block = block_init();
    int blocks_num = tell(fd)/BLOCK_SIZE;

    for(int i=3;i<blocks_num;i++){
        block_get(allocator,fd,i,&block);
        int offset = block->byteArray[0];
        for(int i=0;i<offset;i++){
            Record *record = heap_file_map_record(block,fields,i,size);
            record_print(record);
            record_destroy(record);
        }
        block_unpin(allocator,fd,&block);
    }

    block_destroy(block);
}