// Taken from OBOS.

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "fb.h"
#include "rand.h"

#define OBOS_UNUSED(n) ((void)n)

#ifndef OBOS_TEXT_BACKGROUND
#   define OBOS_TEXT_BACKGROUND 0x00000000
#endif

uintptr_t fw_random()
{
	return mt_random();
}

// void delay_impl(void* userdata)
// {
// 	Core_EventSet((event*)userdata, true);
// }
// static timer* delay(timer_tick ms, timer* cached_t)
// {
// 	// event e = EVENT_INITIALIZE(EVENT_NOTIFICATION);
// 	// timer* t = cached_t ? cached_t : Core_TimerObjectAllocate(NULL);
// 	// memzero(t, sizeof(*t));
// 	// t->handler = delay_impl;
// 	// t->userdata = &e;
// 	// // printf("wait for %d ms\n", ms);
// 	// obos_status status = Core_TimerObjectInitialize(t, TIMER_MODE_DEADLINE, ms*1000);
// 	// OBOS_ASSERT(status == OBOS_STATUS_SUCCESS && "Core_TimerObjectInitialize");
// 	// status = Core_WaitOnObject(WAITABLE_OBJECT(e));
// 	// OBOS_ASSERT(status == OBOS_STATUS_SUCCESS && "Core_WaitOnObject");
// 	// return t;
// 	OBOS_UNUSED(cached_t);
// 	// irql oldIrql = Core_RaiseIrql(IRQL_DISPATCH);
// 	timer_tick deadline = CoreS_GetTimerTick() + CoreH_TimeFrameToTick(ms*1000);
// 	while(CoreS_GetTimerTick() < deadline)
// 		OBOSS_SpinlockHint();
// 	// Core_LowerIrql(oldIrql);
// 	return NULL;
// }
#define delay(ms, unused) (usleep((ms)*1000))
void create_thread(void* entry, uintptr_t udata)
{
	pthread_t thr = {};
	pthread_create(&thr, NULL, entry, (void*)udata);
	pthread_detach(thr);
}
void plot_pixel(uint32_t rgbx, int32_t x, int32_t y)
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	assert(x < (int32_t)FRAMEBUFFER_WIDTH);
	assert(y < (int32_t)FRAMEBUFFER_HEIGHT);
	if (x > (int32_t)FRAMEBUFFER_WIDTH || y > (int32_t)FRAMEBUFFER_HEIGHT)
		return;
	// uint8_t* fb = OBOS_TextRendererState.fb.backbuffer_base ? OBOS_TextRendererState.fb.backbuffer_base : OBOS_TextRendererState.fb.base;
    uint8_t* fb_addr = fb.base;
    fb_addr += (y*fb.pitch+x*fb.bpp/8);
    switch (fb.format)
	{
	case FB_FORMAT_RGB:
		*fb_addr++ = rgbx & 0xff;
		*fb_addr++ = (rgbx >> 8) & 0xff;
		*fb_addr++ = (rgbx >> 16) & 0xff;
		break;
	case FB_FORMAT_BGR:
		*fb_addr++ = (rgbx >> 16) & 0xff;
		*fb_addr++ = (rgbx >> 8) & 0xff;
		*fb_addr++ = rgbx & 0xff;
		break;
	case FB_FORMAT_RGBA:
		*(uint32_t*)fb_addr = rgbx;
		break;
	case FB_FORMAT_ARGB:
		rgbx >>= 8;
		*(uint32_t*)fb_addr = rgbx;
		break;
	default:
		break;
	}
}
uint32_t random_pixel()
{
    return ((fw_random() + 0x808080) & 0xffffff) << 8;
}

typedef struct firework_data
{
	int32_t x,y;
	uint32_t rgbx;
	float act_x, act_y;
	float vel_x, vel_y;
	int explosion_range;
	_Atomic(size_t) refcount;
	bool can_free;
	bool stress_test;
	float direction;
} firework_data;

float fp_rand_sign()
{
	uintptr_t val = fw_random();
	if (fw_random() % 2)
	{
		return -(val/((float)MT_RANDOM_MAX));
	}
	return (val/((float)MT_RANDOM_MAX));
}

_Atomic(size_t) nParticlesLeft = 0;
void particle_handler(void* udata)
{
	firework_data* parent = udata;
	firework_data data = {.x=parent->x,.y=parent->y,.act_x=parent->act_x,.act_y=parent->act_y,.direction=parent->direction,.stress_test = parent->stress_test};
	int ExplosionRange = parent->explosion_range;
	if (!(--parent->refcount) && parent->can_free)
	{
		// OBOS_KernelAllocator->Free(OBOS_KernelAllocator, parent, sizeof(*parent));
		free(parent);
		parent = NULL;
	}
	int Angle = fw_random() % 65536;
	// data.vel_x = fixedpt_mul(cos(Angle), fp_rand_sign())*ExplosionRange;
	// data.vel_y = fixedpt_mul(sin(Angle), fp_rand_sign())*ExplosionRange;
	data.vel_x = cos(Angle) * fp_rand_sign() * ExplosionRange;
	data.vel_y = sin(Angle) * fp_rand_sign() * ExplosionRange;
	const int expires_in = 2000 + fw_random() % 2000;
	data.rgbx = random_pixel();
	int t = 0;
	for (int i = 0; i < expires_in; )
	{
		plot_pixel(data.rgbx, data.x, data.y);
		int curr_delay = 8+(t!=0);
		delay(curr_delay, timer);
		i += curr_delay;
		if (++t == 3)
			t = 0;
		plot_pixel(OBOS_TEXT_BACKGROUND, data.x, data.y);
		float temp_pt = curr_delay / 1000.f;
		data.act_x = data.vel_x * temp_pt + data.act_x;
		data.act_y = data.act_y + data.vel_y * temp_pt;
		data.x = data.act_x;
		data.y = data.act_y;
		if (data.y < 0)
			break;
		if (data.x < 0)
			break;
		if (data.y >= (int32_t)FRAMEBUFFER_HEIGHT)
			break;
		if (data.x >= (int32_t)FRAMEBUFFER_WIDTH)
			break;
		data.vel_y += -10.f * temp_pt;
	}
	die:
	nParticlesLeft--;
	pthread_exit(NULL);
}
static void explodeable_handler(bool stress_test)
{
	firework_data data = {};
	int x_offset = (FRAMEBUFFER_WIDTH*400)/1024;
	data.x = FRAMEBUFFER_WIDTH / 2;
	data.y = FRAMEBUFFER_HEIGHT - 1;
	data.stress_test = stress_test;
	data.act_x = data.x;
	data.act_y = data.y;
	data.vel_y = -(float)(400+fw_random()%400);
	data.direction = fp_rand_sign();
	data.vel_x = x_offset * data.direction;
	data.rgbx = random_pixel();
	data.explosion_range = fw_random() % 100 + 100;
	int expires_in = 500 + fw_random() % 500;
	int t = 0, i = 0;
	for (; i < expires_in; )
	{
		plot_pixel(data.rgbx, data.x, data.y);
		int curr_delay = 16+(t!=0);
		delay(curr_delay, timer);
		i += curr_delay;
		if (++t == 3)
			t = 0;
		plot_pixel(OBOS_TEXT_BACKGROUND, data.x, data.y);
        float temp_pt = curr_delay / 1000.f;
		data.act_x += data.vel_x * temp_pt;
		data.act_y += data.vel_y * temp_pt;
		data.x = data.act_x;
		data.y = data.act_y;
		if (data.y >= (int32_t)FRAMEBUFFER_HEIGHT || data.y < 0)
			break;
		if (data.x >= (int32_t)(FRAMEBUFFER_WIDTH) || data.x < 0)
			break;
		data.vel_y += -10 * temp_pt;
	}
	int nParticles = fw_random() % 100 + 100;
	firework_data* fw_clone = calloc(1, sizeof(firework_data));
	memcpy(fw_clone, &data, sizeof(data));
	nParticlesLeft += nParticles;
	for (int i = 0; i < nParticles; i++)
	{
		fw_clone->can_free = false;
		fw_clone->refcount++;
		create_thread((void*)particle_handler, (uintptr_t)fw_clone);
	}
	fw_clone->can_free = true;
	pthread_exit(NULL);
}
static void SpawnNewExplodable(bool stress_test)
{
	create_thread((void*)explodeable_handler, stress_test);
}

uintptr_t random_seed();

void TestDriver_Fireworks(uint32_t max_iterations, int spawn_min, int spawn_max, bool stress_test)
{
	mt_seed(random_seed());
    for (uint32_t y = 0; y < fb.height; y++)
        for (uint32_t x = 0; x < fb.width; x++)
            plot_pixel(OBOS_TEXT_BACKGROUND, x,y);
    for (uint32_t i = 0; i < max_iterations; i++)
	{
		int nToSpawn = fw_random() % spawn_max + spawn_min;
		for (int i = 0; i < nToSpawn; i++)
			SpawnNewExplodable(stress_test);
		// Wait 1-2 seconds to spawn new fireworks.
		delay(fw_random() % 2000 + 2000, t);
		// while (threadCounts[last_direction])
		// 	OBOSS_SpinlockHint();
	}
}

extern uint64_t random_seed_x86_64();
uintptr_t random_seed()
{
    uint64_t seed = random_seed_x86_64();
    if (!seed)
        seed = time(NULL);
    return seed;
}
asm (
    "\
    .intel_syntax noprefix;\
    .global random_seed_x86_64;\
    random_seed_x86_64:;\
    push rbp; mov rbp, rsp;\
    .rdrand:;\
	    mov eax, 1;\
	    xor ecx,ecx;\
	    cpuid;\
	    bt ecx, 30;\
	    jnc .rdseed;\
	    rdrand rax;\
	    jnc .rdrand;\
	    jmp .done;\
    .rdseed:;\
    	mov eax, 7;\
    	xor ecx,ecx;\
    	cpuid;\
    	bt ebx, 18;\
    	jnc .done;\
    	rdseed rax;\
    	jnc .rdseed;\
    .done:;\
    leave; ret;\
    .att_syntax prefix;\
    "
);
