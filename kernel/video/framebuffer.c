#include <stdint.h>

static uint32_t* fb;
static uint32_t width;
static uint32_t height;
static uint32_t pitch;

void fb_init(uint64_t addr, uint32_t w, uint32_t h, uint32_t p) {
    fb = (uint32_t*)(uintptr_t)addr;
    width = w;
    height = h;
    pitch = p;
}

void fb_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int iy = y; iy < y + h; iy++) {
        for (int ix = x; ix < x + w; ix++) {
            fb[iy * pitch + ix] = color;
        }
    }
}