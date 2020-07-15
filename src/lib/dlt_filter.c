/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author
 * Shengli Hu <eshenhu@gmail.com>
 *
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_filter.c
 */

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__WIN32__)
#include <syslog.h> /* for LOG_... */
#endif

#include "dlt_common.h"
#include "dlt_filter.h"
#include "dlt_user_cfg.h"
#include "dlt_util.h"

typedef struct
{
    DltFltFuncBase base;
    int data;
} DltFltFuncIntEQ;

typedef struct
{
    DltFltFuncBase base;
    int lower;
    int upper;
} DltFltFuncIntBTW;

typedef struct
{
    DltFltFuncBase base;
    char chr;
} DltFltFuncCharEQ;

typedef struct
{
    DltFltFuncBase base;
    char *cmp;
} DltFltFuncStrEQ;

static int32_t dlt_flt_reg_num(void);

bool isFLTIntCmp(struct DltFltFuncBase *base, void *d);
bool isFLTIntBtw(struct DltFltFuncBase *base, void *d);
bool isFLTCharCmp(struct DltFltFuncBase *base, void *d);
//static bool isFLTStrCmp(struct DltFltFuncBase *base, void *d);
//static bool isFLTStrReg(DltFltFuncBase *base, void *d);

static void dlt_flt_init_a_slot(DltFltItem *cur);

bool isFLTIntCmp(struct DltFltFuncBase *base, void *d) {
    DltFltFuncIntEQ *s = (DltFltFuncIntEQ *)base;
    return s->data == *(int *)d ? true : false;
}

DltFltFuncIntEQ *dlt_flt_new_struct_int_eq() {
    DltFltFuncIntEQ *ptr = malloc(sizeof(DltFltFuncIntEQ));
    ptr->base.type = DLT_FLT_SUP_FUNC_INT_EQ;
    ptr->base.func = isFLTIntCmp;
    ptr->data = 0;
    return ptr;
}

bool isFLTIntBtw(struct DltFltFuncBase *base, void *d) {
    int v = *(int*)d;
    DltFltFuncIntBTW *s = (DltFltFuncIntBTW *)base;
    return (v >= s->lower) && (v <= s->upper);
}

DltFltFuncIntBTW *dlt_flt_new_struct_int_btw() {
    DltFltFuncIntBTW *ptr = malloc(sizeof(DltFltFuncIntBTW));
    ptr->base.type = DLT_FLT_SUP_FUNC_INT_BTW;
    ptr->base.func = isFLTIntBtw;
    ptr->lower = 0;
    ptr->upper = 0;
    return ptr;
}

bool isFLTCharCmp(struct DltFltFuncBase *base, void *d) {
    DltFltFuncCharEQ *s = (DltFltFuncCharEQ *)base;
    return s->chr == *(char*)d;
}

DltFltFuncCharEQ *dlt_flt_new_struct_char() {
    DltFltFuncCharEQ *ptr = malloc(sizeof(DltFltFuncCharEQ));
    ptr->base.type = DLT_FLT_SUP_FUNC_CHAR_EQ;
    ptr->base.func = isFLTCharCmp;
    return ptr;
}

DltFltItem **dlt_flt_alloc() {
    DltFltItem **hdr = calloc(1, sizeof(DltFltItem *[DLT_FLT_SIZE]));
    return hdr;
}

void dlt_flt_init_a_slot(DltFltItem *cur) {
    memset(cur->uuid, 0, sizeof(cur->uuid));
    cur->uuid_hash = -1;
    cur->type = DLT_FLT_SUP_NONE;
    for (int j = 0; j < DLT_FLT_FUNC_SIZE; ++j)
        cur->func_vldt[j] = NULL;
}

void dlt_flt_init(DltFltItem *hdr[]) {
    for (uint32_t i = 0; i < DLT_FLT_SIZE; ++i) {
        hdr[i] = malloc(sizeof(DltFltItem));
        dlt_flt_init_a_slot(hdr[i]);
    }
}

void dlt_flt_termination(DltFltItem *hdr[]) {
    DltFltItem *cur = hdr[0];
    for (uint32_t i = 0; i < DLT_FLT_SIZE; ++i, cur = hdr[i]) {
        memset(cur->uuid, 0, sizeof(cur->uuid));
        cur->uuid_hash = -1;
        cur->type = DLT_FLT_SUP_NONE;
        for (int j = 0; j < DLT_FLT_FUNC_SIZE; ++j) {
            if (cur->func_vldt[j]) {
                free(cur->func_vldt[j]);
                cur->func_vldt[j] = NULL;
            }
        }
        free(cur);
        cur = NULL;
    }
    free(hdr);
}

static int32_t reg_idx = 0;
// global variable
int32_t dlt_flt_reg_num() {
    assert((reg_idx < DLT_FLT_SIZE) && "exceed the allowed max value");
    dlt_vlog(LOG_INFO, "Reg slot with %d\n", reg_idx);
    return reg_idx++;
}

// Test only for
void dlt_flt_reset_reg_num() {
    reg_idx = 0;
}
/**
 * @brief 
 * 
 */
DltFltFuncBase **dlt_flt_find_func_slot(DltFltFuncBase *hdr[]) {
    for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
        if (!hdr[i])
            return &(hdr[i]);
    }
    return NULL;
}

void dlt_flt_reset_func_slot(DltFltFuncBase *hdr[]) {
    for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
        if (hdr[i]) {
            free(hdr[i]);
            hdr[i] = NULL;
        }
    }
}


int32_t dlt_flt_reg_dep(DltFltItem * const hdr[], const char* uuid, DltFltSupEnum e) {
    int32_t idx = dlt_flt_reg_num();
    DltFltItem *slot = hdr[idx];

    int size = strlen(uuid);
    if (size > DLT_FLT_UUID_MAX)
        return -1;

    strncpy(slot->uuid, uuid, size);
    slot->uuid_hash = hash(uuid);
    slot->type = e;

    for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
        slot->func_vldt[i] = NULL;
    }
    return idx;
}

int32_t dlt_flt_reg_int(DltFltItem *const hdr[], const char *uuid) {
    return dlt_flt_reg_dep(hdr, uuid, DLT_FLT_SUP_INT);
}

int32_t dlt_flt_reg_char(DltFltItem *const hdr[], const char *uuid) {
    return dlt_flt_reg_dep(hdr, uuid, DLT_FLT_SUP_CHAR);
}

int32_t dlt_flt_reg_str(DltFltItem *const hdr[], const char *uuid) {
    return dlt_flt_reg_dep(hdr, uuid, DLT_FLT_SUP_STR);
}

bool dlt_flt_attach_int_eq(DltFltItem *hdr[], int32_t idx, int v) {
    DltFltItem *slot = hdr[idx];

    if (slot->type != DLT_FLT_SUP_INT) {
        dlt_log(LOG_ERR, "No expected type as DLT_FLT_SUP_INT!\n");
        return false;
    }

    DltFltFuncBase **base = dlt_flt_find_func_slot(slot->func_vldt);
    if (!base) {
        dlt_log(LOG_ERR, "Failed to find a free func slot!\n");
        return false;
    }

    DltFltFuncIntEQ *ptr = dlt_flt_new_struct_int_eq();
    if (!ptr) {
        dlt_log(LOG_ERR, "Failed to malloc for func slot!\n");
        return false;
    }

    ptr->data = v;
    *base = (DltFltFuncBase *)ptr;

    return true;
}

bool dlt_flt_attach_int_btw(DltFltItem *hdr[], int32_t idx, int lower, int upper) {
    if (lower >= upper) {
        dlt_vlog(LOG_ERR, "ERR lower=%d should lower than upper=%d!\n", lower, upper);
        return false;
    }

    DltFltItem *slot = hdr[idx];

    if (slot->type != DLT_FLT_SUP_INT) {
        dlt_log(LOG_ERR, "No expected type as DLT_FLT_SUP_INT!\n");
        return false;
    }
    
    DltFltFuncBase **base = dlt_flt_find_func_slot(slot->func_vldt);
    if (!base) {
        dlt_log(LOG_ERR, "Failed to find a free func slot!\n");
        return false;
    }

    DltFltFuncIntBTW *ptr = dlt_flt_new_struct_int_btw();
    if (!ptr) {
        dlt_log(LOG_ERR, "Failed to malloc for func slot!\n");
        return false;
    }

    ptr->lower = lower;
    ptr->upper = upper;
    *base = (DltFltFuncBase *)ptr;

    return true;
}

bool dlt_flt_attach_char(DltFltItem *hdr[], int32_t idx, char v) {
    DltFltItem *slot = hdr[idx];

    if (slot->type != DLT_FLT_SUP_CHAR) {
        dlt_log(LOG_ERR, "No expected type as DLT_FLT_SUP_INT!\n");
        return false;
    }

    DltFltFuncBase **base = dlt_flt_find_func_slot(slot->func_vldt);
    if (!base) {
        dlt_log(LOG_ERR, "Failed to find a free func slot!\n");
        return false;
    }

    DltFltFuncCharEQ *ptr = dlt_flt_new_struct_char();
    if (!ptr) {
        dlt_log(LOG_ERR, "Failed to malloc for func slot!\n");
        return false;
    }

    ptr->chr = v;
    *base = (DltFltFuncBase *)ptr;

    return true;    
}

void dlt_flt_detach_all(DltFltItem *hdr[], int32_t idx) {
    DltFltItem *slot = hdr[idx];
    dlt_flt_reset_func_slot(slot->func_vldt);
}

// Runtime check
bool dlt_flt_is_allow_dep(DltFltItem *hdr[], DltFltSupEnum e, uint32_t idx, void* data) {
    if (idx >= DLT_FLT_SIZE)
        return false;

    if (hdr[idx]->type != e)
        return false;

    int i = 0;
    for (; i < DLT_FLT_FUNC_SIZE; ++i) {
        DltFltFuncBase *ptr = hdr[idx]->func_vldt[i];
        if (!ptr)
            break;

        if (ptr->func(ptr, data))
            return true;
    }

    return i == 0 ? true : false;
}
bool dlt_flt_is_allow_int(DltFltItem *hdr[], uint32_t idx, int data) {
    return dlt_flt_is_allow_dep(hdr, DLT_FLT_SUP_INT, idx, &data);
}

bool dlt_flt_is_allow_char(DltFltItem *hdr[], uint32_t idx, char chr) {
    return dlt_flt_is_allow_dep(hdr, DLT_FLT_SUP_CHAR, idx, &chr);
}

bool dlt_flt_is_allow_str(DltFltItem *hdr[], uint32_t idx, char *str) {
    return dlt_flt_is_allow_dep(hdr, DLT_FLT_SUP_STR, idx, str);
}