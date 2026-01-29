#include <stdint.h>

extern void fb_init();
extern void fb_draw_rect(int x, int y, int w, int h, uint32_t color);

void kernel_main() {
    fb_init();

    // фон
    fb_draw_rect(0, 0, 1024, 768, 0x00111111);

    // логотип-блок
    fb_draw_rect(412, 300, 200, 80, 0x00FFFFFF);

    while (1) {
        __asm__ volatile("wfe");
    }
}