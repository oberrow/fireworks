#include <stdint.h>

#define FRAMEBUFFER_WIDTH (fb.width)
#define FRAMEBUFFER_HEIGHT (fb.height)
enum fb_format
{
	FB_FORMAT_RGBA,
	FB_FORMAT_ARGB,
	FB_FORMAT_RGB,
	FB_FORMAT_BGR,
};
struct fb {
	uint8_t* base;
	uint8_t bpp;
	uint32_t pitch, height, width;
    enum fb_format format;
} extern fb;