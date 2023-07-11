#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "./compiler/tokenizer.h"
#include "./compiler/parser.h"
#include "./compiler/semantic.h"
#include "./disk/block.h"
#include "./db_files/heapfile.h"
#include "./db_files/db_file.h"

DBFile* load_db_file(char* filepath) {
    DBFile *db_file = malloc(sizeof(DBFile));

    db_file->allocator = block_allocator_initialize(BUFFER_SIZE*1024);
    db_file->table = NULL;
    db_file->fd = -1;

    //open heap file
    HPErrorCode status = heap_file_open(&db_file->allocator,filepath,&db_file->fd,&db_file->table);
    if(status==HP_FILE_ERROR) printf("INCORRECT FILE\n");

    return db_file;
}

GList* load_db(char* dir_path) {
    GList *files = NULL;
    GDir *dir;
    const gchar *filename;
    dir = g_dir_open(dir_path, 0, NULL);
    while ((filename = g_dir_read_name(dir))) {
        if (strrchr(filename, '.') == NULL) { // checks for the existence of '.' in the filename
            GString *filepath = g_string_new ("./data/");
            g_string_append (filepath, filename);
            files = g_list_append (files, load_db_file (filepath->str));
            g_string_free (filepath, FALSE);
        }
    }
    return files;
}

int main(int argc, char **argv){
    GList *files = load_db ("./data");

//    for (GList *lp = files; lp != NULL; lp = lp->next) {
//        DBFile *db_file = lp->data;
//        table_print (db_file->table, NULL);
//    }

    gchar *query = "SELECT customer.name FROM customer";

    printf("Executing query:\n\t%s\t\n", query);

    GList *tokens = tokenize(query);
    Statement *stm = parse_statement(&tokens);
    gboolean is_valid = smemantic_analyze (stm, files);

//    if (is_valid)
//        printf("\nValid query!\n");
//    else
//        printf("\nInvalid query!\n");

    return is_valid;
}