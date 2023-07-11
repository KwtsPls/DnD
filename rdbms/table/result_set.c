#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record.h"
#include "result_set.h"


//Function to create a result set
ResultSet *result_set_create(){
    ResultSet *set = malloc(sizeof(ResultSet));
    set->tables=NULL;
    set->results=NULL;
    return set;
}

//Function to initialize a result item
ResultItem *result_item_create(){
    ResultItem *item = malloc(sizeof(ResultItem));
    item->records=NULL;
    return item;
}

//Function to add a table to the result set
ResultSet *result_set_add_table(ResultSet *set,Table *table){
    if(g_list_find(set->tables,table)==NULL)
        set->tables = g_list_append(set->tables,table);
    return set;
}

//Function to add a table to the result set
ResultSet *result_set_add_item(ResultSet *set,ResultItem *item){
    set->results = g_list_append(set->results,item);
    return set;
}

//Function to get the indexes (i,j) for a record and its field
void result_set_get_indexes(ResultSet *set,Table *table,char *field,int *i,int *j){
    *i = g_list_index(set->tables,table);
    *j = table_field_pos(table,field);
}

//Function to get the value of a record's field based on indexes
void *result_item_get_value(ResultItem *item,int i,int j){
    Record *record = (Record*)g_list_nth(item->records,i)->data;
    return record_get_field_value(record,j);
}

//Function to add a record for a different table to the item
ResultItem *result_item_add_record(ResultItem *item,Record *record){
    item->records = g_list_append(item->records,record);
    return item;
}

//Function to perform an AND operation on two result sets
ResultSet *result_set_and(ResultSet *set1,ResultSet *set2){
    //Check the result set tables to find the index of the intersection
    ResultSet *set = result_set_create();
    GList *indexes = NULL;
    int i=0;
    for (GList *lp1 = set1->tables; lp1 != NULL; lp1 = lp1->next) {
        for (GList *lp2 = set2->tables; lp2 != NULL; lp2 = lp2->next) {
            Table *t1 = lp1->data;
            Table *t2 = lp2->data;
            if (strcmp(t1->name,t2->name)==0) {
                set = result_set_add_table(set,(Table*)lp1->data);
                int *data = malloc(sizeof(int));
                *data = i;
                indexes = g_list_append(indexes,data);
                break;
            }
        }
        i++;
    }

    //if there is no common table - perform a cross join
    if(set->tables==NULL){
        set->tables = g_list_copy(set1->tables);
        set->tables = g_list_concat(set->tables,g_list_copy(set2->tables));

        for (GList *lp1 = set1->results; lp1 != NULL; lp1 = lp1->next) {
            for (GList *lp2 = set2->results; lp2 != NULL; lp2 = lp2->next){
                ResultItem *item1 = (ResultItem*)lp1->data;
                ResultItem *item2 = (ResultItem*)lp2->data;
                ResultItem *item = result_item_create();
                item->records = g_list_copy_deep(item1->records,record_copy,NULL);
                item->records = g_list_concat(item->records,g_list_copy_deep(item2->records,record_copy,NULL));
                result_set_add_item(set,item);
            }
        }
        return set;
    }
    else{
        set->tables = g_list_concat(g_list_copy(set2->tables),set->tables);

        for (GList *lp1 = set1->results; lp1 != NULL; lp1 = lp1->next) {
            for (GList *lp2 = set2->results; lp2 != NULL; lp2 = lp2->next){
                ResultItem *item1 = (ResultItem*)lp1->data;
                ResultItem *item2 = (ResultItem*)lp2->data;
                ResultItem *item = result_item_intersection(item1,item2,indexes);
                if(item!=NULL){
                    set = result_set_add_item(set,item);
                    break;
                }
            }
        }

        result_set_destroy(set1);
        result_set_destroy(set2);
        g_list_free_full(indexes,free);
        return set;
    }
}


//Function to perform an intersection on two result items
ResultItem *result_item_intersection(ResultItem *item1,ResultItem *item2,GList *indexes){
    int check;
    for (GList *lp1 = indexes; lp1 != NULL; lp1 = lp1->next) {
        check=1;
        int i = *(int*)lp1->data;
        GList *node = g_list_nth(item1->records,i);
        Record *r = (Record*)node->data;

        if(result_item_check_record(item2,r)==0){
            check=0;
            break;
        }
    }

    if(check==0){
        return NULL;
    }
    else{
        ResultItem *item = result_item_create();
        item->records = g_list_copy_deep(item2->records, record_copy, NULL);
        item = result_item_get_records(item,item1);
        return item;
    }
}

//Function to check if a record exists in a record list
int result_item_check_record(ResultItem *item,Record *r){
    for (GList *lp1 = item->records; lp1 != NULL; lp1 = lp1->next) {
        if(record_compare(r,(Record*)lp1->data)==1) return 1;
    }
    return 0;
}

//Function to insert the records of a result item to another
ResultItem *result_item_get_records(ResultItem *item1,ResultItem *item2){
    for (GList *lp1 = item2->records; lp1 != NULL; lp1 = lp1->next) {
        Record *r = lp1->data;
        if(result_item_check_record(item1,r)==0){
            item1->records = g_list_append(item1->records, record_copy(r,NULL));
        }
    }
    return item1;
}


/*********** OPERATIONS ON THE RESULT SET *****************************/

/*########## HELPER FUNCTIONS FOR HASH TABLE ENTRIES ###################*/

//Function to create a result entry
ResultEntry *result_entry_create(ResultItem *item,int i,int j){
    ResultEntry *entry = malloc(sizeof(ResultEntry));
    entry->item = item;
    entry->i = i;
    entry->j = j;
    return entry;
}

//Function to hash a result entry
unsigned int result_entry_hash(const void *_entry){
    ResultEntry *entry = (ResultEntry*)_entry;
    DataBox *databox = result_item_get_value(entry->item,entry->i,entry->j);
    char *str=NULL;
    if(databox->type==INT_BOX){
        str=malloc(10);
        memset(str,0,10);
        sprintf(str,"%d",*(int*)databox->data);
    }
    else if(databox->type==DOUBLE_BOX){
        str=malloc(20);
        memset(str,0,20);
        sprintf(str,"%f",*(double*)databox->data);
    }
    else{
        str = strdup((char*)databox->data);
    }

    char *temp = str;

    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    free(temp);
    return hash;
}

void result_value_nothing(void *nothing){
    return;
}

//Function to destroy a result value for the hash table
void result_value_destroy(void *_list){
    GList *list = (GList*)_list;
    g_list_free_full(list,result_value_nothing);
}

//Function to compare two result entry
int result_entry_equals(const void *_a,const void *_b){
    ResultEntry *ra = (ResultEntry*)_a;
    ResultEntry *rb = (ResultEntry*)_b;

    DataBox *databox1 = result_item_get_value(ra->item,ra->i,ra->j);
    DataBox *databox2 = result_item_get_value(rb->item,rb->i,rb->j);

    if(databox_compare(databox1,databox2)==0) return TRUE;
    else return FALSE;
}

/*###########################################################################*/

//Function to prepare the result according to the variables in the select
GList *result_set_finalize(ResultSet *set,Statement *statement,GList *db_files){
    GList *result=NULL;

    //check if the statement has a group by field
    if(statement->group!=NULL){
        Var *var = statement->group;

        //Find the table involved in the group by
        Table *table;
        for(GList *lp = db_files; lp != NULL; lp = lp->next){
            DBFile *dbFile = (DBFile*)lp->data;
            if(strcmp(var->table,dbFile->table->name)==0){
                table = dbFile->table;
                break;
            }
        }

        //Get the indexes for the group by variable
        int i,j;
        result_set_get_indexes(set,table,var->column,&i,&j);

        //Build a hashtable based on the uniqueness of the given field
        GHashTable *group_table = g_hash_table_new_full(result_entry_hash, result_entry_equals, free, result_value_destroy);
        for (GList *lp = set->results; lp != NULL; lp = lp->next) {
            ResultEntry *key = result_entry_create(lp->data,i,j);
            GList *value = g_hash_table_lookup(group_table,key);

            GList *value_list = NULL;
            if(value==NULL){
                value_list = g_list_append(value_list,key->item);
            }
            else{
                value_list = g_list_copy(value);
                value_list = g_list_append(value_list,key->item);
            }
            g_hash_table_insert(group_table,key,value_list);
        }

        GList *keys = g_hash_table_get_keys(group_table);
        GList *values = g_hash_table_get_values(group_table);
        while(keys!=NULL){
            ResultEntry *record_entry = (ResultEntry*)keys->data;
            GList *group_values = (GList*)values->data;

            Record *r = record_create();
            for(GList *lp = statement->vars; lp != NULL; lp = lp->next){
                Var *result_var = (Var*)lp->data;

                //Find the table involved in the group by
                for(GList *lp1 = db_files; lp1 != NULL; lp1 = lp1->next){
                    DBFile *dbFile = (DBFile*)lp1->data;
                    if(strcmp(result_var->table,dbFile->table->name)==0){
                        table = dbFile->table;
                        break;
                    }
                }
                //Get the indexes for the group by variable
                result_set_get_indexes(set,table,result_var->column,&i,&j);

                if(result_var->type==VAR){
                    DataBox *databox = result_item_get_value(record_entry->item,i,j);

                    DataBox *copy = databox_copy(databox,NULL);
                    r = record_add_field(r,copy);
                }
                else{

                    DataBox *aggr_entry=NULL;
                    for(GList *node = group_values; node != NULL; node = node->next){
                        ResultItem *item = (ResultItem*)node->data;
                        DataBox *databox = result_item_get_value(item,i,j);
                        if(result_var->type==COUNT) aggr_entry = databox_count(aggr_entry);
                        else aggr_entry = databox_sum(aggr_entry,databox);
                    }
                    r = record_add_field(r,aggr_entry);
                }
            }

            result = g_list_append(result,r);
            keys = keys->next;
            values = values->next;
        }
    }
    else{

        int has_aggr=0;
        Table *table;
        int i,j;
        for (GList *node = set->results; node != NULL; node = node->next) {
            ResultItem *item = (ResultItem*)node->data;
            Record *r = record_create();
            for(GList *lp = statement->vars; lp != NULL; lp = lp->next){
                Var *result_var = (Var*)lp->data;

                //Find the table involved in the group by
                for(GList *lp1 = db_files; lp1 != NULL; lp1 = lp1->next){
                    DBFile *dbFile = (DBFile*)lp1->data;
                    if(strcmp(result_var->table,dbFile->table->name)==0){
                        table = dbFile->table;
                        break;
                    }
                }
                //Get the indexes for the group by variable
                result_set_get_indexes(set,table,result_var->column,&i,&j);

                if(result_var->type==VAR){
                    DataBox *databox = result_item_get_value(item,i,j);

                    DataBox *copy = databox_copy(databox,NULL);
                    r = record_add_field(r,copy);
                }
                else{

                    DataBox *aggr_entry=NULL;
                    for(GList *node1 = set->results; node1 != NULL; node1 = node1->next){
                        ResultItem *itemAggr = (ResultItem*)node1->data;
                        DataBox *databox = result_item_get_value(itemAggr,i,j);
                        if(result_var->type==COUNT) aggr_entry = databox_count(aggr_entry);
                        else aggr_entry = databox_sum(aggr_entry,databox);
                    }
                    r = record_add_field(r,aggr_entry);
                    has_aggr=1;
                }
            }

            result = g_list_append(result,r);
            if(has_aggr==1)
                break;
        }


    }

    //check if the result is to be sorted
    if(statement->order!=NULL){
        Var *order_var = statement->order->var;
        int isAsc = statement->order->isAsc;

        //Find the table involved in the group by
        int i=0;
        for(GList *node=statement->vars; node != NULL; node = node->next){
            Var *var = (Var*)node->data;
            if(strcmp(var->table,order_var->table)==0 && strcmp(var->column,order_var->column)==0)
                break;
            i++;
        }

        if(isAsc==1)
            result = g_list_sort_with_data(result,record_compare_asc,&i);
        else
            result = g_list_sort_with_data(result, record_compare_desc,&i);
    }


    //check if limit exists
    if(statement->limit>0 && statement->limit< g_list_length(result)){
        GList *limit = NULL;
        GList *head = result;
        for(int i=0;i<statement->limit;i++){
            Record *r = (Record*)head->data;
            limit = g_list_append(limit, record_copy(r,NULL));
            head = head->next;
        }

        g_list_free_full(result, record_destroy);
        result = limit;
    }

    result_set_destroy(set);
    return result;
}



void result_set_table_free(void *table){
    return;
}

//Function to delete a result
void result_set_destroy(void *_set){
    ResultSet *set = (ResultSet*)_set;
    g_list_free_full(set->tables,result_set_table_free);
    g_list_free_full(set->results,result_item_destroy);
    free(set);
}

//Function to delete a result item
void result_item_destroy(void *_item){
    ResultItem *item = (ResultItem*)_item;
    g_list_free_full(item->records, record_destroy);
    free(item);
}


//Function to delete a result set but not the records
void result_set_destroy_soft(void *_set){
    ResultSet *set = (ResultSet*)_set;
    g_list_free_full(set->tables,result_set_table_free);
    g_list_free_full(set->results,result_item_destroy_soft);
    free(set);
}

//Function to delete a result item but not the records
void result_item_destroy_soft(void *_item){
    ResultItem *item = (ResultItem*)_item;
    g_list_free_full(item->records, result_set_table_free);
    free(item);
}

//Function to delete a result set but not the records
void result_set_destroy_soft_1(void *_set){
    ResultSet *set = (ResultSet*)_set;
    g_list_free_full(set->tables,result_set_table_free);
    g_list_free_full(set->results,result_item_destroy_soft_1);
    free(set);
}

//Function to delete a result item but not the records
void result_item_destroy_soft_1(void *_item){
    ResultItem *item = (ResultItem*)_item;
    free(item);
}