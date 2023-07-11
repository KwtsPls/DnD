#ifndef DND_BNL_H
#define DND_BNL_H

#include "../compiler/tokenizer.h"
#include "../disk/block.h"
#include "../memory/buffer_manager.h"
#include "../table/databox.h"
#include "../table/record.h"
#include "../table/table.h"
#include "../table/result_set.h"
#include "../db_files/heapfile.h"

//Function to perform a block nested loop join on heap files
ResultSet *heap_file_bnl(BlockAllocator **allocator,Table **table1,Table **table2,char *field1,char *field2,TokenType op);

#endif //DND_BNL_H
