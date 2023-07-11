#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bnl.h"

//Function to perform a block nested loop join on heap files
ResultSet *heap_file_bnl(BlockAllocator **allocator,Table **table1,Table **table2,char *field1,char *field2,TokenType op){
    int field_pos1= table_field_pos(*table1,field1);
    int field_pos2= table_field_pos(*table2,field2);

    int fd1 = (*table1)->fd;
    int fd2 = (*table2)->fd;

    GList *f1 = (*table1)->fields;
    GList *f2 = (*table2)->fields;

    int left_block_num = tell(fd1)/BLOCK_SIZE;
    int right_block_num = tell(fd2)/BLOCK_SIZE;

    int size1 = (*table1)->record_size;
    int size2 = (*table2)->record_size;

    //Inner table must have fewer blocks
    if(right_block_num>left_block_num){
        int t = fd1;
        fd1 = fd2;
        fd2 = t;

        t = left_block_num;
        left_block_num = right_block_num;
        right_block_num = t;

        t = size1;
        size1 = size2;
        size2 = t;

        t = field_pos1;
        field_pos1 = field_pos2;
        field_pos2 = t;

        GList *lt = f1;
        f1 = f2;
        f2 = lt;
    }

    //Create a new result set for the join
    ResultSet *set = result_set_create();
    set = result_set_add_table(set,*table1);
    set = result_set_add_table(set,*table2);

    Block *block1 = block_init();
    Block *block2 = block_init();

    for(int i=3;i<left_block_num;i++){
        block_get(allocator,fd1,i,&block1);
        char offset1 = block1->byteArray[0];
        for(int j=3;j<right_block_num;j++){
            block_get(allocator,fd2,j,&block2);
            int offset2 = block2->byteArray[0];
            for(int t1=0;t1<offset1;t1++){
                for(int t2=0;t2<offset2;t2++){
                    Record *tr = heap_file_map_record(block1,f1,t1,size1);
                    Record *ts = heap_file_map_record(block2,f2,t2,size2);
                    int join_result = databox_compare_value(record_get_field_value(tr,field_pos1),
                                                            ((DataBox*)record_get_field_value(ts,field_pos2))->data);
                    if(result_condition(join_result,op)==1){
                        ResultItem *item = result_item_create();
                        item = result_item_add_record(item,tr);
                        item = result_item_add_record(item,ts);
                        set = result_set_add_item(set,item);
                    }
                    else{
                        record_destroy(ts);
                        record_destroy(tr);
                    }
                }
            }
            block_unpin(allocator,fd2,&block2);
        }
        block_unpin(allocator,fd1,&block1);
    }

    block_destroy(block1);
    block_destroy(block2);

    return set;
}