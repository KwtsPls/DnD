#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./disk/block.h"
#include "./memory/priority_queue.h"
#include "./memory/buffer_manager.h"

int main(){
    BlockAllocator *allocator = block_allocator_initialize(1536);
    Block *block = block_init();

    block_file_create("temp");
    int fd = block_file_open(&allocator,"temp");

    block = block_allocate(&allocator,fd,block);

    block = block_allocate(&allocator,fd,block);
    strcpy(block->byteArray,"AB");
    block = block_set_dirty(block);
    block_unpin(&allocator,fd,&block);

    block = block_allocate(&allocator,fd,block);
    strcpy(block->byteArray,"DG");
    block = block_set_dirty(block);
    block_unpin(&allocator,fd,&block);

    pq_print(allocator->bufferManager->pq);

    block_get(&allocator,fd,0,&block);
    block_unpin(&allocator,fd,&block);

    pq_print(allocator->bufferManager->pq);

    block_allocator_destroy(allocator);
    block_destroy(block);
    return 0;
}

