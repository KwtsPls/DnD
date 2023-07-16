#ifndef DND_HEAPFILE_H
#define DND_HEAPFILE_H

#include "../compiler/tokenizer.h"
#include "../disk/block.h"
#include "../memory/buffer_manager.h"
#include "../table/databox.h"
#include "../table/record.h"
#include "../table/table.h"
#include "../table/result_set.h"

typedef struct result_set ResultSet;

typedef enum hp_error_codes{HP_OK,
                            HP_FILE_ERROR,
                            HP_CLOSE_ERROR,
                            HP_RECORD_ERROR,
                            HP_FILTER_FIELD_ERROR}HPErrorCode;

//Function to create a heap file as a database file
HPErrorCode heap_file_create(BlockAllocator **allocator, char *filename, char *table_name, GList *fields, GList *field_names);
//Function to open a heap file
HPErrorCode heap_file_open(BlockAllocator **allocator,char *filename,int *file_descriptor,Table **table);
//Function to insert a record in the heap file
HPErrorCode heap_file_insert(BlockAllocator **allocator,Record *record,int fd);
//Function to close a heap file
HPErrorCode heap_file_close(BlockAllocator **allocator,int fd,Table **table);

/*************** DATABASE OPERATIONS ON HEAP FILES *****************/
//Perform a filter operation on the records of the table
ResultSet *heap_file_filter(BlockAllocator **allocator,Table **table,char *field,TokenType op,void *value);
//Function to return all records in the table
ResultSet *heap_file_scan(BlockAllocator **allocator,Table **table);

/************ HELPER FUNCTIONS ******************/
//Function to write a record inside a block
void heap_file_write_record(Block *block,Record *r,int offset);
//Function to map fields to a record
Record *heap_file_map_record(Block *block,GList *fields,int offset,int size);
//Function to print all records in the file
void heap_file_print_all(BlockAllocator **allocator,int fd,GList *fields,int size);

#endif //DND_HEAPFILE_H
