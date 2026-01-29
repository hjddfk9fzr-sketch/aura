#include <stdint.h>
#include "bootinfo.h"

extern void fb_init(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch);
extern void fb_draw_rect(int x, int y, int w, int h, uint32_t color);

void kernel_main(void* bootinfo) {
    struct BootInfo* bi = (struct BootInfo*)bootinfo;

    fb_init(bi->fb.base, bi->fb.width, bi->fb.height, bi->fb.pixels_per_scanline);

    // фон
    fb_draw_rect(0, 0, 1024, 768, 0x00111111);

    // логотип-блок
    fb_draw_rect(412, 300, 200, 80, 0x00FFFFFF);

    while (1) {
        __asm__ volatile("wfe");
    }
}