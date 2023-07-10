#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <glib.h>
#include "heapfile.h"

//Function to create a heap file as a database file
HPErrorCode heap_file_create(BlockAllocator **allocator, char *filename, char *table_name, GList *fields, GList *field_names){

    //Create a block file
    block_file_create(filename);
    int fd = block_file_open(allocator,filename);

    //Create a file holding metadata for the current table
    char *metadata_file = malloc(strlen(filename)+10);
    memset(metadata_file,0, strlen(filename)+10);
    sprintf(metadata_file,"%s.metadata",filename);

    creat(metadata_file,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    int mfd = open(metadata_file,O_RDWR);
    for(int i=0;i<g_list_length(field_names);i++) {
        char *field_name = (char*)g_list_nth(field_names,i)->data;
        char *line = malloc(strlen(field_name)+2);
        memset(line,0, strlen(field_name)+2);
        sprintf(line,"%s\n",field_name);
        write(mfd,line, strlen(line));
        free(line);
    }
    free(metadata_file);
    close(mfd);

    //Initialize a block structure to begin disk operations
    Block *block = block_init();

    block = block_allocate(allocator,fd,block);
    //At the start of the first block - write heap file descriptor
    int heap_descriptor = 17;
    memcpy(block->byteArray,&heap_descriptor,sizeof(int));

    int fields_num = g_list_length(fields);
    memcpy(block->byteArray + sizeof(int), &fields_num, sizeof(int));

    unsigned long long records_num = 0;
    memcpy(block->byteArray + sizeof(int)*2,&records_num, sizeof(unsigned long long));

    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Second block of the file holds only the name of the table
    block = block_allocate(allocator,fd,block);
    strcpy(block->byteArray,table_name);

    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Third block contains (field_type,field_size_ pairs
    block = block_allocate(allocator,fd,block);
    for(int i=0;i<fields_num;i++){
        DataBox *databox = (DataBox*)g_list_nth(fields,i)->data;
        int type = databox->type;
        int size = databox->size;


        memcpy(block->byteArray + sizeof(int)*i*2,&type, sizeof(int));
        memcpy(block->byteArray + sizeof(int) + sizeof(int)*i*2,&size, sizeof(int));
    }
    block = block_set_dirty(block);
    block_unpin(allocator,fd,&block);

    //Initialization is over - close file
    block_file_close(allocator,fd);
    block_destroy(block);

    return HP_OK;
}

//Function to open a heap file
HPErrorCode heap_file_open(BlockAllocator **allocator,char *filename,int *file_descriptor,Table **table){
    Block *block = block_init();
    int fd = block_file_open(allocator,filename);

    //Check if current file is heap file
    block_get(allocator,fd,0,&block);
    int heap_descriptor;
    memcpy(&heap_descriptor,block->byteArray, sizeof(int));

    block_unpin(allocator,fd,&block);
    block_destroy(block);

    if(heap_descriptor==17){
        *file_descriptor = fd;

        //Initialize the table object for the current heap file
        *table = table_init();

        //save the fd of the file
        (*table)->fd = fd;

        block = block_init();

        block_get(allocator,fd,0,&block);
        //get the number of fields for the table records
        int num_fields;
        memcpy(&num_fields,block->byteArray + sizeof(int), sizeof(int));
        block_unpin(allocator,fd,&block);

        //get the table name
        block_get(allocator,fd,1,&block);
        (*table)->name = strdup(block->byteArray);
        block_unpin(allocator,fd,&block);

        //get the field statistics
        block_get(allocator,fd,2,&block);
        GList *fields = NULL;
        for(int i=0;i<num_fields;i++){
            DataBox *databox=NULL;

            int type;
            int size;

            memcpy(&type,block->byteArray + sizeof(int)*i*2, sizeof(int));
            memcpy(&size,block->byteArray + sizeof(int) + sizeof(int)*i*2, sizeof(int));

            databox = databox_create(NULL,size,type);
            fields = g_list_append(fields,databox);
            (*table)->record_size += size;
        }
        (*table)->fields = fields;
        block_unpin(allocator,fd,&block);

        //save the filename in the table
        (*table)->filename = strdup(filename);

        //get the field names from the metadata file
        char *metadata_file = malloc(strlen(filename)+10);
        memset(metadata_file,0, strlen(filename)+10);
        sprintf(metadata_file,"%s.metadata",filename);
        (*table)->metadata = metadata_file;

        FILE * fp;
        char * line = NULL;
        size_t len = 0;

        GList *field_names=NULL;
        fp = fopen(metadata_file, "r");
        while (getline(&line, &len, fp) != -1){
            line[strlen(line)-1]='\0';
            char *field_name = strdup(line);
            field_names = g_list_append(field_names,field_name);
        }
        (*table)->field_names = field_names;


        if(line!=NULL) free(line);
        fclose(fp);
        block_destroy(block);
        return HP_OK;
    }
    else{
        *file_descriptor=-1;
        block_file_close(allocator,fd);
        return HP_FILE_ERROR;
    }
}

//Function to insert a record in the heap file
HPErrorCode heap_file_insert(BlockAllocator **allocator,Record *record,int fd){
    //Get the number of blocks in the file
    int blocks_num = tell(fd)/BLOCK_SIZE;
    Block *block=block_init();

    //Heap file is not empty - check if last block has enough space
    int empty=1;
    if(blocks_num>3){
        block_get(allocator,fd,blocks_num-1,&block);
        //check the record counter in the block
        if(block->byteArray[0] < (BLOCK_SIZE/record->size)){
            int offset = block->byteArray[0];
            heap_file_write_record(block,record,offset);
            offset++;
            memset(block->byteArray,offset, sizeof(char));
            empty=0;
            block_set_dirty(block);
        }
        block_unpin(allocator,fd,&block);
    }

    //Heap file is either empty or last block is full - allocate new block
    if(empty==1){
        block = block_allocate(allocator,fd,block);
        //set the record counter to 1
        memset(block->byteArray,1, sizeof(char));
        heap_file_write_record(block,record,0);
        block_set_dirty(block);
        block_unpin(allocator,fd,&block);
    }

    block_destroy(block);
    return HP_OK;
}

//Function to close a heap file
HPErrorCode heap_file_close(BlockAllocator **allocator,int fd,Table **table){
    int status = block_file_close(allocator,fd);
    table_destroy(*table);
    *table = NULL;
    if(status==-1) return HP_CLOSE_ERROR;
    else return HP_OK;
}

/*************** DATABASE OPERATIONS ON HEAP FILES *****************/

//Perform a filter operation on the records of the table
ResultSet *heap_file_filter(BlockAllocator **allocator,Table **table,char *field,TokenType op,void *value){
    int field_pos= table_field_pos(*table,field);
    if(field_pos==-1) return NULL;

    int fd = (*table)->fd;

    //Get the number of blocks in the file
    int blocks_num = tell(fd)/BLOCK_SIZE;
    Block *block = block_init();

    //Create a result stream
    ResultSet *set = result_set_create();
    //Add the table for this filter
    set = result_set_add_table(set,*table);

    for(int i=3;i<blocks_num;i++){
        block_get(allocator,fd,i,&block);
        int offset = block->byteArray[0];
        for(int j=0;j<offset;j++){
            Record *r = heap_file_map_record(block,(*table)->fields,j,(*table)->record_size);
            int filter_result = databox_compare_value(record_get_field_value(r,field_pos),value);
            if(result_condition(filter_result,op)==1){
                ResultItem *item = result_item_create();
                item = result_item_add_record(item,r);
                set = result_set_add_item(set,item);
            }

        }
        block_unpin(allocator,fd,&block);
    }

    block_destroy(block);
    return set;
}

/************ HELPER FUNCTIONS ******************/
//Function to write a record inside a block
void heap_file_write_record(Block *block,Record *r,int offset){
    int n = g_list_length(r->list);
    int sum = 0;
    for(int i=0;i<n;i++){
        DataBox *databox = (DataBox*)g_list_nth(r->list,i)->data;
        memcpy(block->byteArray + sizeof(char) + r->size * offset + sum,databox->data, databox->size );
        sum += databox->size;
    }
}

//Function to map fields to a record
Record *heap_file_map_record(Block *block,GList *fields,int offset,int size){
    Record *record = record_create();

    int sum=0;
    int n = g_list_length(fields);
    for(int i=0;i<n;i++){
        DataBox *field = (DataBox*)g_list_nth(fields,i)->data;

        void *data = malloc(field->size);
        memcpy(data, block->byteArray + sizeof(char) + offset * size + sum, field->size);
        DataBox *databox = databox_create(data,field->size,field->type);
        record = record_add_field(record,databox);
        sum += field->size;
    }

    return record;
}

//Function to print all records in the file
void heap_file_print_all(BlockAllocator **allocator,int fd,GList *fields,int size){
    Block *block = block_init();
    int blocks_num = tell(fd)/BLOCK_SIZE;

    for(int i=3;i<blocks_num;i++){
        block_get(allocator,fd,i,&block);
        int offset = block->byteArray[0];
        for(int j=0;j<offset;j++){
            Record *record = heap_file_map_record(block,fields,j,size);
            record_print(record);
            record_destroy(record);
        }
        block_unpin(allocator,fd,&block);
    }

    block_destroy(block);
}