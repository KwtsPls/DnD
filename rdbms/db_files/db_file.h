#ifndef _DB_FILE_H_
#define _DB_FILE_H_

typedef struct block_allocator BlockAllocator;
typedef struct table Table;

typedef struct {
    BlockAllocator *allocator;
    Table *table;
    int fd;
}  DBFile;

#endif //_DB_FILE_H_
