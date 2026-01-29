#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <stdint.h>

struct Framebuffer {
    uint64_t base;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
    uint32_t pixel_format; // reserved
};

struct BootInfo {
    struct Framebuffer fb;
};

#endif // BOOTINFO_H
