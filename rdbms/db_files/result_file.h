#ifndef DND_RESULT_FILE_H
#define DND_RESULT_FILE_H

#include "../compiler/tokenizer.h"
#include "../disk/block.h"
#include "../memory/buffer_manager.h"
#include "../table/databox.h"
#include "../table/record.h"
#include "../table/table.h"

typedef enum rf_error_codes{RF_OK,
                            RF_CLOSE_ERROR,
                            RF_FILE_ERROR}RFErrorCode;

//Function to create a result file
RFErrorCode result_file_create(BlockAllocator **allocator,char *filename, char *table1, char *field1,char *table2, char *field2);
//Function to open a result file
RFErrorCode result_file_open(BlockAllocator **allocator, char *filename, int *file_descriptor);
//Function to close a heap file
RFErrorCode result_file_close(BlockAllocator **allocator,int fd);

#endif //DND_RESULT_FILE_H
