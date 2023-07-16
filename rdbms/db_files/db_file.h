#ifndef _DB_FILE_H_
#define _DB_FILE_H_

#include <glib.h>
#include "../table/table.h"
#include "../disk/block.h"
#include "heapfile.h"

typedef struct db{
    GList *tables;
    BlockAllocator *allocator;
}Database;

//Function to initialize a database
Database *database_open(char* dir_path);
Database *database_open_existing(Database *db, char* dir_path);
//Function to execute a query - returns a list of records
GList *database_query(Database *db,char *query);
//Function to close a database
void database_close(Database *db);
/************ HELPER FUNCTIONS ***********************************/
Table *database_get_table(Database *db,char *name);
//Function that gets an expression and parses if it is a filter or join operation
int decide_expression(Database *db,Expr *expr,Table **table1,void **value,char **field1,char **field2,TokenType *op);

#endif //_DB_FILE_H_
