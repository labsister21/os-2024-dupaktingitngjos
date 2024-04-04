#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"

// void kernel_setup(void) {
//     uint32_t a;
//     uint32_t volatile b = 0x0000BABE;
//     __asm__("mov $0xCAFE0000, %0" : "=r"(a));
//     while (true) b += 1;
// }
//
// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     while (true);
// }

// void kernel_setup(void)
// {
//     framebuffer_clear();
//     framebuffer_write(3, 8, 'A', 0, 0xA);
//     framebuffer_write(3, 9, 'B', 0, 0xB);
//     framebuffer_write(3, 10, 'C', 0, 0xC);
//     framebuffer_write(3, 11, 'D', 0, 0xD);
//     framebuffer_write(3, 12, 'E', 0, 0xE);
//     framebuffer_write(3, 13, 'F', 0, 0xF);
//     framebuffer_write(3, 14, '0', 0, 0x0);
//     framebuffer_write(3, 15, '1', 0, 0x1);
//     framebuffer_write(3, 16, '2', 0, 0x2);
//     framebuffer_write(3, 17, '3', 0, 0x3);
//     framebuffer_write(3, 18, '4', 0, 0x4);
//     framebuffer_write(3, 19, '5', 0, 0x5);
//     framebuffer_write(3, 20, '6', 0, 0x6);
//     framebuffer_write(3, 21, '7', 0, 0x7);
//     framebuffer_write(3, 22, '8', 0, 0x8);
//     framebuffer_write(3, 23, '9', 0, 0x9);

//     framebuffer_set_cursor(3, 10);
//     while (true)
//         ;
// }

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    __asm__("int $0x4");
    while (true);
}
