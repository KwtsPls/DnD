#ifndef DND_HEAPFILE_H
#define DND_HEAPFILE_H

#include "../disk/block.h"
#include "../memory/buffer_manager.h"
#include "../table/databox.h"
#include "../table/record.h"

typedef enum hp_error_codes{HP_OK,HP_FILE_ERROR,HP_CLOSE_ERROR}HPErrorCode;

//Function to create a heap file as a database file
HPErrorCode heap_file_create(BlockAllocator **allocator, char *filename, char *table_name, GList *fields);
//Function to open a heap file
HPErrorCode heap_file_open(BlockAllocator **allocator,char *filename,int *file_descriptor);
//Function to insert a record in the heap file
HPErrorCode heap_file_insert(BlockAllocator **allocator,Record *record,int fd);
//Function to close a heap file
HPErrorCode heap_file_close(BlockAllocator **allocator,int fd);

/************ HELPER FUNCTIONS ******************/
//Function to write a record inside a block
void heap_file_write_record(Block *block,Record *r,int offset);
//Function to map fields to a record
Record *heap_file_map_record(Block *block,GList *fields,int offset,int size);
//Function to print all records in the file
void heap_file_print_all(BlockAllocator **allocator,int fd,GList *fields,int size);

#endif //DND_HEAPFILE_H
