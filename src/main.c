#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "fb.h"

#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

struct fb fb = {};
void TestDriver_Fireworks(uint32_t max_iterations, int spawn_min, int spawn_max, bool stress_test);
int main()
{
    struct fb_fix_screeninfo fix_screeninfo = {};
    struct fb_var_screeninfo screeninfo = {};
    int fd = open("/dev/fb0", O_RDWR);
    ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo);
    ioctl(fd, FBIOGET_FSCREENINFO, &fix_screeninfo);
    fb.width = screeninfo.xres;
    fb.height = screeninfo.yres;
    fb.bpp = screeninfo.bits_per_pixel;
    fb.pitch = fb.width*fb.bpp/8; // TODO: Pitch
    bool use_linear = false;
    uint32_t r_shift = screeninfo.red.offset;
    uint32_t g_shift = screeninfo.green.offset;
    uint32_t b_shift = screeninfo.blue.offset;
    uint32_t x_shift = screeninfo.transp.offset;
    do {
        if (fb.bpp == 24)
        {
            if (b_shift == 0 && g_shift == 8 && r_shift == 16)
                fb.format = FB_FORMAT_RGB;
            if (r_shift == 0 && g_shift == 8 && b_shift == 16)
                fb.format = FB_FORMAT_BGR;
//            printf("framebuffer is %s\n", fb.format == FB_FORMAT_RGB ? "RGB" : "BGR");
            break;
        } 
        else if (fb.bpp == 32) 
        {
            if (x_shift == 0 && b_shift == 8 && g_shift == 16 && r_shift == 24)
                fb.format = FB_FORMAT_RGBA;
            if (b_shift == 0 && g_shift == 8 && r_shift == 16 && x_shift == 24)
                fb.format = FB_FORMAT_ARGB;
//            printf("framebuffer is %s\n", fb.format == FB_FORMAT_ARGB ? "ARGB" : "RGBA");
            break;
        }
        fprintf(stderr, "could not identify framebuffer format of /dev/fb0\n");
        return -1;
    } while(0);
    int mem_fd = fd;
    fb.base = (uint8_t*)mmap(NULL, fix_screeninfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0);
    if ((uintptr_t)fb.base == UINTPTR_MAX)
    {
        perror("mmap");
        return errno;
    }
//    printf ("Framebuffer is %dx%dx%d\n", fb.width, fb.height, fb.bpp);
    TestDriver_Fireworks(UINT32_MAX, 2, 5, true);
}
