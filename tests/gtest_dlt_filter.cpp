/*
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
 * \author Shengli Hu <eshenhu@gmail.com>
 *
 * License MPL-1.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file gtest_dlt_filter.cpp
 */

#include "gtest/gtest.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__WIN32__)
#include <syslog.h> /* for LOG_... */
#endif

extern "C" {
#include "dlt_filter.h"
#include "dlt_user.h"
#include "dlt_user_cfg.h"
}

/* TEST COMMENTED OUT WITH */
/* TODO: */
/* DO FAIL! */

/* tested functions */
/*
 * DltFltItem **dlt_flt_alloc();
 * void dlt_flt_init(DltFltItem *const hdr[]);
 * void dlt_flt_termination(DltFltItem *const hdr[]);
 */

/*
 * #define DLT_FLT_REG_INT(NAME) dlt_flt_reg_int(hdr, NAME)
 * int32_t dlt_flt_reg_int(DltFltItem * const hdr[], char* uuid);
 * bool dlt_flt_attach_int_eq(DltFltItem *hdr[], int32_t idx, int64_t v);
 * bool dlt_flt_attach_int_btw(DltFltItem *hdr[], int32_t idx, int64_t lower, int64_t upper);
 * 
 * //int32_t dlt_flt_reg_char(DltFltItem * const hdr[], char* uuid);
 * //int32_t dlt_flt_reg_str(DltFltItem * const hdr[], char* uuid);
 */

/*
 * #define DLT_FLT_I(IDX, VAR)  dlt_flt_is_allow_int(hdr, IDX, VAR)
 * bool dlt_flt_is_allow_int(const DltFltItem *hdr[], uint32_t idx, int data);
 * 
 * #define DLT_FLT_C(IDX, VAR) dlt_flt_is_allow_char(hdr, IDX, VAR)
 * bool dlt_flt_is_allow_char(const DltFltItem *hdr[], uint32_t idx, char chr);
 * 
 * #define DLT_FLT_S(IDX, VAR) dlt_flt_is_allow_str(hdr, IDX, VAR)
 * bool dlt_flt_is_allow_str(const DltFltItem *hdr[], uint32_t idx, char* str)
 * 
 */

int get_num_of_active_rules(DltFltItem *p) {
    int cnt = 0;
    for (uint32_t i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
        if (p->func_vldt[i])
            ++cnt;
    }
    return cnt;
}

/*/////////////////////////////////////// */
/* start initial dlt */
TEST(t_dlt_init, onetime) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    dlt_flt_termination(p);
}

/* alloc one rule */
TEST(t_dlt_reg, normal) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        EXPECT_EQ(0, dlt_flt_reg_int(p, "TEST:1"));

        for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
            EXPECT_TRUE(dlt_flt_attach_int_eq(p, 0, i));
            EXPECT_EQ(i + 1, get_num_of_active_rules(p[0]));
        }
        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 0, 6));
        EXPECT_EQ(DLT_FLT_FUNC_SIZE, get_num_of_active_rules(p[0]));

        dlt_flt_detach_all(p, 0);
        EXPECT_EQ(0, get_num_of_active_rules(p[0]));

        // add it again
        for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
            EXPECT_TRUE(dlt_flt_attach_int_eq(p, 0, i));
            EXPECT_EQ(i + 1, get_num_of_active_rules(p[0]));
        }
        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 0, 6));
        EXPECT_EQ(DLT_FLT_FUNC_SIZE, get_num_of_active_rules(p[0]));

        // add another.
        EXPECT_EQ(1, dlt_flt_reg_char(p, "TEST:2"));

        // try to add a int
        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 1, 6));

        // 'a' 'b' 'c' 'd' ...
        for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
            EXPECT_TRUE(dlt_flt_attach_char(p, 1, 'a' + i));
            EXPECT_EQ(i + 1, get_num_of_active_rules(p[1]));
        }

        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 1, 'x'));
        EXPECT_EQ(DLT_FLT_FUNC_SIZE, get_num_of_active_rules(p[1]));

        // add another.
        EXPECT_EQ(2, dlt_flt_reg_bitmask(p, "TEST:3"));

        // try to add a int
        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 2, 6));

        // 'a' 'b' 'c' 'd' ...
        for (int i = 0; i < DLT_FLT_FUNC_SIZE; ++i) {
            EXPECT_TRUE(dlt_flt_attach_bitmask(p, 2, 0x01 << i));
            EXPECT_EQ(i + 1, get_num_of_active_rules(p[2]));
        }

        EXPECT_FALSE(dlt_flt_attach_int_eq(p, 1, 0x10));
        EXPECT_EQ(DLT_FLT_FUNC_SIZE, get_num_of_active_rules(p[2]));
    }
    dlt_flt_termination(p);
}

/* alloc more rules*/
TEST(t_dlt_reg, add_more) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        dlt_flt_reset_reg_num();
        char name[DLT_FLT_UUID_MAX + 1] = {0};
        for (int i = 0; i < DLT_FLT_SIZE; ++i) {
            sprintf(name, "TEST:%d", i);
            EXPECT_EQ(i, dlt_flt_reg_int(p, (const char *)&name));
            EXPECT_TRUE(dlt_flt_attach_int_eq(p, i, i));
            EXPECT_EQ(1, get_num_of_active_rules(p[i]));
        }

        //crash will be happened here for catch this.
        //EXPECT_EQ(-1, dlt_flt_reg_int(p, "NO_POSSIBLE"));
    }
    dlt_flt_termination(p);
}

/* alloc more rules*/
TEST(t_dlt_reg, abnormal) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        dlt_flt_reset_reg_num();
        EXPECT_EQ(-1, dlt_flt_reg_int(p, "0123456789ABCDEFG"));
    }
    dlt_flt_termination(p);
}

/* alloc more rules*/
TEST(t_dlt_reg, validate_rules_int) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        dlt_flt_reset_reg_num();
        EXPECT_EQ(0, dlt_flt_reg_int(p, "0123456789ABCDE"));

        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 1234));

        EXPECT_TRUE(dlt_flt_attach_int_eq(p, 0, 9));
        EXPECT_EQ(1, get_num_of_active_rules(p[0]));

        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 1234));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 9));

        EXPECT_TRUE(dlt_flt_attach_int_eq(p, 0, 8));
        EXPECT_EQ(2, get_num_of_active_rules(p[0]));

        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 1234));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 9));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 8));
        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 3));

        // 1~5, 8, 9
        EXPECT_TRUE(dlt_flt_attach_int_btw(p, 0, 1, 5));
        EXPECT_EQ(3, get_num_of_active_rules(p[0]));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 9));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 8));
        EXPECT_TRUE(dlt_flt_is_allow_int(p, 0, 3));
        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 6));
        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 6000));

        //
        EXPECT_FALSE(dlt_flt_attach_int_btw(p, 0, 10, 5));
        EXPECT_EQ(3, get_num_of_active_rules(p[0]));
    }
    dlt_flt_termination(p);
}

/* alloc more rules*/
TEST(t_dlt_reg, validate_rules_char) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        dlt_flt_reset_reg_num();
        EXPECT_EQ(0, dlt_flt_reg_char(p, "TEST:CHAR"));

        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 1234));
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'a'));

        EXPECT_TRUE(dlt_flt_attach_char(p, 0, 'a'));
        EXPECT_EQ(1, get_num_of_active_rules(p[0]));

        // 'a'
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'a'));
        EXPECT_FALSE(dlt_flt_is_allow_char(p, 0, 'b'));

        // 'a' 'a' it should be OK; Check by user.
        EXPECT_TRUE(dlt_flt_attach_char(p, 0, 'a'));
        EXPECT_EQ(2, get_num_of_active_rules(p[0]));

        // 'a'
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'a'));
        EXPECT_FALSE(dlt_flt_is_allow_char(p, 0, 'b'));

        // 'a' 'b'
        EXPECT_TRUE(dlt_flt_attach_char(p, 0, 'b'));
        EXPECT_EQ(3, get_num_of_active_rules(p[0]));

        // 'a' 'b'
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'a'));
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'b'));
        EXPECT_FALSE(dlt_flt_is_allow_char(p, 0, 'c'));

        // 'a' 'b'
        EXPECT_TRUE(dlt_flt_attach_char(p, 0, 'b'));
        EXPECT_EQ(4, get_num_of_active_rules(p[0]));

        // 'a' 'b'
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'a'));
        EXPECT_TRUE(dlt_flt_is_allow_char(p, 0, 'b'));
        EXPECT_FALSE(dlt_flt_is_allow_char(p, 0, 'c'));
    }
    dlt_flt_termination(p);
}

/* alloc more rules*/
TEST(t_dlt_reg, validate_rules_bitmask) {
    DltFltItem **p = dlt_flt_alloc();
    EXPECT_NE(nullptr, p);

    dlt_flt_init(p);
    {
        dlt_flt_reset_reg_num();
        EXPECT_EQ(0, dlt_flt_reg_bitmask(p, "TEST:BITMASK"));

        EXPECT_FALSE(dlt_flt_is_allow_int(p, 0, 1234));
        EXPECT_TRUE(dlt_flt_is_allow_bitmask(p, 0, 0x01));

        EXPECT_TRUE(dlt_flt_attach_bitmask(p, 0, 0x01));

        EXPECT_TRUE(dlt_flt_is_allow_bitmask(p, 0, 0x01));
        EXPECT_FALSE(dlt_flt_is_allow_bitmask(p, 0, 0x02));
        EXPECT_TRUE(dlt_flt_is_allow_bitmask(p, 0, 0x03));
        EXPECT_EQ(1, get_num_of_active_rules(p[0]));
    }
    dlt_flt_termination(p);
}

/* start initial dlt */
TEST(t_dlt_reg, onetime) {
    /**
     * Unset DLT_USER_ENV_LOG_MSG_BUF_LEN environment variable
     * to make sure the dlt user buffer initialized with default value
     */
    unsetenv(DLT_USER_ENV_LOG_MSG_BUF_LEN);
    EXPECT_EQ(DLT_RETURN_OK, dlt_init());
}

/* Nomal usage */
TEST(t_dlt_reg, usermode) {
    DltContext context;

    EXPECT_LE(DLT_RETURN_OK, dlt_register_app("TUSR", "dlt_filter.c tests"));
    EXPECT_LE(DLT_RETURN_OK,
              dlt_register_context(&context, "TEST", "dlt_filter.c context test"));

    // you don't need do this in normal application.
    dlt_flt_reset_reg_num();

    uint32_t FLT_C1 = DLT_FLT_REG_CHAR("TEST:CHR");
    EXPECT_EQ(0, FLT_C1);

    EXPECT_TRUE(DLT_FLT_C(FLT_C1, 'a'));
    EXPECT_TRUE(DLT_FLT_C(FLT_C1, 'b'));
    EXPECT_FALSE(DLT_FLT_I(FLT_C1, 1234));

    EXPECT_TRUE(DLT_FLT_ATTACH_CHAR(FLT_C1, 'a'));
    EXPECT_TRUE(DLT_FLT_C(FLT_C1, 'a'));
    EXPECT_FALSE(DLT_FLT_C(FLT_C1, 'b'));
    EXPECT_FALSE(DLT_FLT_I(FLT_C1, 1234));

    DLT_FLT_LOG(DLT_FLT_C(FLT_C1, 'a'), context, DLT_LOG_INFO, DLT_STRING("DLT Filter Example: SHOULD be print"));
    DLT_FLT_LOG(DLT_FLT_C(FLT_C1, 'c'), context, DLT_LOG_INFO, DLT_STRING("DLT Filter Example: SHOULD NOT be print"));

    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_context(&context));
    EXPECT_LE(DLT_RETURN_OK, dlt_unregister_app());
}
/*/////////////////////////////////////// */
/* main */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
