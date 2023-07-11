#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <glib.h>
#include "parser.h"

gboolean smemantic_analyze (Statement *stm, GList *tables);

#endif //_SEMANTIC_H_
