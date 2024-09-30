#pragma once

#include <stdint.h>

#if UINTPTR_MAX == UINT32_MAX
#define STATE_SIZE  624
#define MIDDLE      397
#define INIT_SHIFT  30
#define INIT_FACT   1812433253
#define TWIST_MASK  0x9908b0df
#define SHIFT1      11
#define MASK1       0xffffffff
#define SHIFT2      7
#define MASK2       0x9d2c5680
#define SHIFT3      15
#define MASK3       0xefc60000
#define SHIFT4      18
#else
#define STATE_SIZE  312
#define MIDDLE      156
#define INIT_SHIFT  62
#define TWIST_MASK  0xb5026f5aa96619e9
#define INIT_FACT   6364136223846793005
#define SHIFT1      29
#define MASK1       0x5555555555555555
#define SHIFT2      17
#define MASK2       0x71d67fffeda60000
#define SHIFT3      37
#define MASK3       0xfff7eee000000000
#define SHIFT4      43
#endif
#define MT_RANDOM_MAX UINTPTR_MAX

uintptr_t mt_random(void);
void mt_seed(uintptr_t seed);
uintptr_t random_seed();