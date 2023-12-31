#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "db_file.h"
#include "../compiler/semantic.h"
#include "../joins/bnl.h"
#include "../joins/hash_join.h"


Table* load_db_file(char* filepath,BlockAllocator **allocator) {
    //open heap file
    int fd;
    Table *table=NULL;

    HPErrorCode status = heap_file_open(allocator,filepath,&fd,&table);
    if(status==HP_FILE_ERROR) printf("INCORRECT FILE\n");

    return table;
}

GList* load_db(char* dir_path,BlockAllocator **allocator) {
    GList *tables = NULL;
    GDir *dir;
    const gchar *filename;
    dir = g_dir_open(dir_path, 0, NULL);
    printf("%s\n", dir_path);
    while ((filename = g_dir_read_name(dir))) {
        if (strrchr(filename, '.') == NULL) { // checks for the existence of '.' in the filename
            GString *filepath = g_string_new (dir_path);
            g_string_append (filepath, filename);
            tables = g_list_append (tables, load_db_file (filepath->str,allocator));
            g_string_free (filepath, TRUE);
        }
    }
    g_dir_close(dir);
    return tables;
}

GList* load_db_temp(char* dir_path,BlockAllocator **allocator) {
    GList *tables = NULL;
    GDir *dir;
    const gchar *filename;
    dir = g_dir_open(dir_path, 0, NULL);
    printf("%s\n", dir_path);
    while ((filename = g_dir_read_name(dir))) {
        if (strrchr(filename, '.') == NULL) { // checks for the existence of '.' in the filename
            GString *filepath = g_string_new (dir_path);
            g_string_append (filepath, filename);
            GString *temp_filepath = g_string_new (filepath->str);
            g_string_append (temp_filepath, ".temp");

            pid_t pid = fork();
            if (pid == 0) { /* child */
                execl("/bin/cp", "/bin/cp", filepath->str, temp_filepath->str, (char *)0);
            }
            while (wait(NULL) > 0);

            tables = g_list_append (tables, load_db_file (temp_filepath->str,allocator));
            g_string_free (filepath, TRUE);
            g_string_free (temp_filepath, TRUE);
        }
    }
    g_dir_close(dir);
    return tables;
}

void database_table_destroy(void *_table){
    return;
}

/************ MAIN DATABASE FUNCTIONALITY ************************/

//Function to initialize a database
Database *database_open(char* dir_path){
    Database *db = malloc(sizeof(Database));
    BlockAllocator *allocator = block_allocator_initialize(BUFFER_SIZE*1024);
    GList *tables = load_db_temp(dir_path,&allocator);

    db->allocator = allocator;
    db->tables = tables;

    return db;
}


//Function to add records of new tables to the database
Database *database_open_existing(Database *db, char* dir_path){
    GList *new_tables = load_db(dir_path,&db->allocator);
    for(GList *node = new_tables; node != NULL; node = node->next){
        Table *table_new = node->data;
        Table *table_old = database_get_table(db,table_new->name);
        heap_file_merge(&db->allocator,&table_old,&table_new);
        heap_file_close(&db->allocator, table_new->fd,&table_new);
    }

    g_list_free_full(new_tables,database_table_destroy);
    return db;
}

//Function to execute a query - returns a list of records
GList *database_query(Database *db,char *query){

    //Compiler part of the query pipeline
    GList *tokens = tokenize(query);
    Statement *stm = parse_statement(&tokens);
    if(stm==NULL){
        g_list_free_full(tokens, token_free);
        return NULL;
    }

    gboolean is_valid = smemantic_analyze(stm, db->tables);

    if(!is_valid){
        statement_free(stm);
        g_list_free_full(tokens, token_free);
        return NULL;
    }

    BooleanExpr *bool = stm->expr;
    ResultSet *set = NULL;

    if(bool!=NULL) {
        for (int i = 0; i < bool->size; i++) {
            Expr *expr = bool->expr[i];
            Table *table1;
            void *data;
            char *field1;
            char *field2;
            TokenType op;
            int join = decide_expression(db, expr, &table1, &data, &field1, &field2, &op);

            //Perform a filter
            if (set == NULL) {
                if (join == 0)
                    set = heap_file_filter(&db->allocator, &table1, field1, op, data);
                else {
                    Table *table2 = (Table *) data;
                    if(op==EQUAL || op==NOT_EQUAL)
                        set = heap_file_hash_join(&db->allocator, &table1, &table2, field1, field2, op);
                    else
                        set = heap_file_bnl(&db->allocator, &table1, &table2, field1, field2, op);
                }
            } else {
                ResultSet *set1 = NULL;
                if (join == 0)
                    set1 = heap_file_filter(&db->allocator, &table1, field1, op, data);
                else {
                    Table *table2 = (Table *) data;
                    if(op==EQUAL || op==NOT_EQUAL)
                        set1 = heap_file_hash_join(&db->allocator, &table1, &table2, field1, field2, op);
                    else
                        set1 = heap_file_bnl(&db->allocator, &table1, &table2, field1, field2, op);
                }
                set = result_set_and(set, set1);
            }

        }
    }
    else{
        for(GList *node = stm->tables; node != NULL; node = node->next){
            TableToken *tableToken = node->data;
            Table *table = database_get_table(db,tableToken->name);

            if(set==NULL)
                set = heap_file_scan(&db->allocator,&table);
            else
                set = result_set_and(set, heap_file_scan(&db->allocator,&table));
        }
    }

    GList *records = result_set_finalize(set,stm,db->tables);

    statement_free(stm);
    g_list_free_full(tokens, token_free);
    return records;
}

//Function to close a database
void database_close(Database *db){
    for(GList *node = db->tables; node != NULL ; node = node->next){
        Table *table = (Table*)node->data;
        heap_file_close(&db->allocator,table->fd,&table);
    }
    g_list_free_full(db->tables,database_table_destroy);
    block_allocator_destroy(db->allocator);
    free(db);
}

/************ HELPER FUNCTIONS ***********************************/
Table *database_get_table(Database *db,char *name){
    for(GList *node = db->tables; node != NULL ; node = node->next){
        Table *table = (Table*)node->data;
        if(strcmp(table->name,name)==0) return table;
    }
    return NULL;
}

//Function that gets an expression and parses if it is a filter or join operation
int decide_expression(Database *db,Expr *expr,Table **table1,void **value,char **field1,char **field2,TokenType *op){
    *op = expr->op;

    //join
    if(expr->x_type==VAR && expr->y_type==VAR){
        Var *x = (Var*)expr->x;
        Var *y = (Var*)expr->y;

        *field1 = x->column;
        *field2 = y->column;

        *table1 = database_get_table(db,x->table);
        *value = database_get_table(db,y->table);
        return 1;
    }
    //filter
    else{
        *field2=NULL;
        if(expr->x_type==VAR){
            Var *x = (Var*)expr->x;
            *field1 = x->column;
            *table1 = database_get_table(db,x->table);
            *value = expr->y;
        }
        else{
            Var *y = (Var*)expr->y;
            *field1 = y->column;
            *table1 = database_get_table(db,y->table);
            *value = expr->x;
        }
        return 0;
    }
}