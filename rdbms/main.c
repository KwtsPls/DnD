#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./disk/block.h"
#include "./memory/priority_queue.h"
#include "./memory/buffer_manager.h"

int main(){
    BlockAllocator *allocator = block_allocator_initialize(BUFFER_SIZE);
    Block *block = block_init();

    block_file_create("temp");
    int fd = block_file_open(&allocator,"temp");
    block = block_allocate(&allocator,fd,block);
    block = block_allocate(&allocator,fd,block);
    block = block_allocate(&allocator,fd,block);

    block_get(&allocator,fd,1,&block);

    return fd;
}

