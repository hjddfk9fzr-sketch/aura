#include <stdint.h>

static uint32_t* fb;
static int width = 1024;
static int height = 768;

void fb_init() {
    // временно хардкод (UEFI GOP подключим дальше)
    fb = (uint32_t*)0x40000000;
}

void fb_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int iy = y; iy < y + h; iy++) {
        for (int ix = x; ix < x + w; ix++) {
            fb[iy * width + ix] = color;
        }
    }
}