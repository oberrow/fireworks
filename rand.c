#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "rand.h"

// shamelessly stolen from https://osdev.wiki/wiki/Random_Number_Generator#Mersenne_Twister

#define LOWER_MASK  0x7fffffff
#define UPPER_MASK  (~(uintptr_t)LOWER_MASK)
static uintptr_t state[STATE_SIZE];
static size_t index = STATE_SIZE + 1;

void mt_seed(uintptr_t s)
{
    index = STATE_SIZE;
    state[0] = s;
    for (size_t i = 1; i < STATE_SIZE; i++)
        state[i] = (INIT_FACT * (state[i - 1] ^ (state[i - 1] >> INIT_SHIFT))) + i;
}

static void twist(void)
{
    for (size_t i = 0; i < STATE_SIZE; i++)
    {
        uintptr_t x = (state[i] & UPPER_MASK) | (state[(i + 1) % STATE_SIZE] & LOWER_MASK);
        x = (x >> 1) ^ (x & 1? TWIST_MASK : 0);
        state[i] = state[(i + MIDDLE) % STATE_SIZE] ^ x;
    }
    index = 0;
}

uintptr_t mt_random(void)
{
    if (index >= STATE_SIZE)
    {
//        assert(index == STATE_SIZE || !"Generator never seeded");
        if (index == STATE_SIZE)
            mt_seed(random_seed());
        twist();
    }

    uintptr_t y = state[index];
    y ^= (y >> SHIFT1) & MASK1;
    y ^= (y << SHIFT2) & MASK2;
    y ^= (y << SHIFT3) & MASK3;
    y ^= y >> SHIFT4;

    index++;
    return y;
}
