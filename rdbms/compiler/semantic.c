#include <stdio.h>
#include "semantic.h"
#include "../compiler/semantic.h"
#include "../db_files/db_file.h"
#include "../table/table.h"

gboolean smemantic_analyze (Statement *stm, GList *tables)
{
    // FROM clause
    for (GList *lp = stm->tables; lp != NULL; lp = lp->next) {
        gboolean table_exists = FALSE;
        Table *from_table = lp->data;
        for (GList *lpd = tables; lpd != NULL; lpd = lpd->next) {
            Table *db_table = lpd->data;
//            printf("Comparing: %s %s\n", from_table->name, db_table->name);
            if (g_strcmp0 (from_table->name, db_table->name) == 0) {
                table_exists = TRUE;
                break;
            }
        }
        if (table_exists == FALSE) {
            printf("\033[0;31mERROR\033[0;37m: Invalid table (%s) in FROM clause\n", from_table->name);
            return FALSE;
        }
    }
    // SELECT clause
    for (GList *lp = stm->vars; lp != NULL; lp = lp->next) {
        gboolean table_exists = FALSE;
        Var *variable = lp->data;
        for (GList *lpd = tables; lpd != NULL; lpd = lpd->next) {
            Table *db_table = lpd->data;
//            printf("Comparing: %s %s\n", variable->table, db_table->name);
            if (g_strcmp0 (variable->table, db_table->name) == 0) {
                for (GList *lpt = db_table->field_names; lpt != NULL; lpt = lpt->next) {
//                    printf("Comparing: %s %s\n", variable->column, (char*) lpt->data);
                    if (g_strcmp0 (variable->column, lpt->data) == 0) {
                        table_exists = TRUE;
                        goto loop_end;
                    }
                }
            }
        }
        loop_end:
        if (table_exists == FALSE) {
            printf("\033[0;31mERROR\033[0;37m: Invalid variable (%s.%s) in SELECT clause\n", variable->table, variable->column);
            return FALSE;
        }
    }
    return TRUE;
}