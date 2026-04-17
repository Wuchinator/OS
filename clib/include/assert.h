#ifndef ASSERT_H
#define ASSERT_H

#include "stdio.h"

#ifdef NDEBUG
    #define assert(expr) ((void)0)
#else
    #define assert(expr) \
        do { \
            if (!(expr)) { \
                printf("ASSERTION FAILED: %s\n", #expr); \
                printf("         File: %s\n", __FILE__); \
                printf("         Line: %d\n", __LINE__); \
                printf("         Func: %s\n", __func__); \
                abort(); \
            } \
        } while (0)
#endif

#define static_assert _Static_assert

#endif
