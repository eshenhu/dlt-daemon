#ifndef DLT_UTIL_H
#define DLT_UTIL_H

#include "dlt_types.h"

struct DltFltItemStu;

uint64_t hash(const char*);

struct DltFltItemStu** dlt_log_get_filter();

#endif