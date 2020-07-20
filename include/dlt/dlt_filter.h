#ifndef DLT_FILTER_H
#define DLT_FILTER_H

#include "dlt_types.h"
#include "dlt_util.h"
#include <stdbool.h>
/**
 * Filter Rules
 * For cache-friendly we use the fixed size table instead of linked-list.
 */
#define DLT_FLT_UUID_MAX 15 // Make sure this value equal 2^N - 1
#define DLT_FLT_SIZE 16
#define DLT_FLT_FUNC_SIZE 4

typedef enum {
    DLT_FLT_SUP_NONE,
    DLT_FLT_SUP_INT,  /* 0, 1234, */
    DLT_FLT_SUP_CHAR, /* 'r', 'w' */
    DLT_FLT_SUP_STR,   /* "guard_002" , "rules_002" */
    DLT_FLT_SUP_BITMASK
} DltFltSupEnum;

typedef enum {
    DLT_FLT_SUP_FUNC_NONE,
    DLT_FLT_SUP_FUNC_INT_EQ,
    DLT_FLT_SUP_FUNC_INT_BTW,
    DLT_FLT_SUP_FUNC_CHAR_EQ,
    DLT_FLT_SUP_FUNC_STR_EQ,
    DLT_FLT_SUP_FUNC_STR_REG,
    DLT_FLT_SUP_FUNC_BIT_EQ,
} DltFltSupFuncEnum;

//typedef bool (*isFLTMatch)(struct DltFltFuncBase *base, void *);
typedef struct DltFltFuncBase {
    //isFLTMatch func;
    bool (*func)(struct DltFltFuncBase *base, void *);
    DltFltSupFuncEnum type;
} DltFltFuncBase;

typedef struct DltFltItemStu
{
    char uuid[DLT_FLT_UUID_MAX + 1]; /* could be Domain:Action, "SQL:R", "RPC:REQ"*/
    uint64_t uuid_hash;
    DltFltSupEnum type;
    DltFltFuncBase *func_vldt[DLT_FLT_FUNC_SIZE];
} DltFltItem;

/********************************************************/
// Init
DltFltItem **dlt_flt_alloc();
void dlt_flt_init(DltFltItem *hdr[]);
void dlt_flt_termination(DltFltItem *hdr[]);

// REG
void dlt_flt_reset_reg_num();

/********************************************************/
// INT
#define DLT_FLT_REG_INT(NAME) dlt_flt_reg_int(dlt_log_get_filter(), NAME)
int32_t dlt_flt_reg_int(DltFltItem *const hdr[], const char *uuid);

#define DLT_FLT_ATTACH_INT_EQ(IDX, NUM) dlt_flt_attach_int_eq(dlt_log_get_filter(), IDX, NUM)
#define DLT_FLT_ATTACH_INT_BTW(IDX, L, U) dlt_flt_attach_int_eq(dlt_log_get_filter(), IDX, L, U)
bool dlt_flt_attach_int_eq(DltFltItem *hdr[], int32_t idx, int v);
bool dlt_flt_attach_int_btw(DltFltItem *hdr[], int32_t idx, int lower, int upper);

// CHAR
#define DLT_FLT_REG_CHAR(NAME) dlt_flt_reg_char(dlt_log_get_filter(), NAME)
int32_t dlt_flt_reg_char(DltFltItem *const hdr[], const char *uuid);
// Attach to an existed flt.
#define DLT_FLT_ATTACH_CHAR(IDX, CHR) dlt_flt_attach_char(dlt_log_get_filter(), IDX, CHR)
bool dlt_flt_attach_char(DltFltItem *hdr[], int32_t idx, char v);

// BITMASK
#define DLT_FLT_REG_BITMASK(NAME) dlt_flt_reg_bitmask(dlt_log_get_filter(), NAME)
int32_t dlt_flt_reg_bitmask(DltFltItem *const hdr[], const char *uuid);
// Attach
#define DLT_FLT_ATTACH_BITMASK(IDX, BIT) dlt_flt_attach_bitmask(dlt_log_get_filter(), IDX, BIT)
bool dlt_flt_attach_bitmask(DltFltItem *hdr[], int32_t idx, uint64_t bit);

// ToDo:
int32_t dlt_flt_reg_str(DltFltItem *const hdr[], const char *uuid);

void dlt_flt_detach_all(DltFltItem *hdr[], int32_t idx);

/********************************************************/
// Runtime check
#define DLT_FLT_I(IDX, VAR) dlt_flt_is_allow_int(dlt_log_get_filter(), IDX, VAR)
bool dlt_flt_is_allow_int(DltFltItem *hdr[], uint32_t idx, int data);

#define DLT_FLT_C(IDX, VAR) dlt_flt_is_allow_char(dlt_log_get_filter(), IDX, VAR)
bool dlt_flt_is_allow_char(DltFltItem *hdr[], uint32_t idx, char chr);

#define DLT_FLT_S(IDX, VAR) dlt_flt_is_allow_str(dlt_log_get_filter(), IDX, VAR)
bool dlt_flt_is_allow_str(DltFltItem *hdr[], uint32_t idx, char *str);

#define DLT_FLT_B(IDX, VAR) dlt_flt_is_allow_bitmask(dlt_log_get_filter(), IDX, VAR)
bool dlt_flt_is_allow_bitmask(DltFltItem *hdr[], uint32_t idx, uint64_t bitmask);
#endif