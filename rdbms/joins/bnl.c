#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bnl.h"

//Function to perform a block nested loop join on heap files
HPErrorCode heap_file_bnl(BlockAllocator **allocator,Table **table1,Table **table2,TokenType op){
    int fd1 = (*table1)->fd;
    int fd2 = (*table2)->fd;


}
