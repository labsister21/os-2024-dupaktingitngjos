#include <stdint.h>
#include <stdbool.h>
#include "ch0/gdt/gdt.h"
#include "ch0/kernel-entrypoint/kernel-entrypoint.h"
#include "ch1/framebuffer/framebuffer.h"
#include "ch1/portio/portio.h"
#include "ch1/idt/idt.h"
#include "ch1/interrupt/interrupt.h"
#include "ch1/keyboard/keyboard.h"
#include "ch1/disk/disk.h"
#include "ch1/fat32/fat32.h"
#include "ch2/paging/paging.h"

// void kernel_setup(void) {
//     uint32_t a;
//     uint32_t volatile b = 0x0000BABE;
//     __asm__("mov $0xCAFE0000, %0" : "=r"(a));
//     while (true) b += 1;
// }

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

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     __asm__("int $0x4");
//     while (true);
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     activate_keyboard_interrupt();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();

//     /* ----------
//         Keyboard
//        ---------- */
//     int col = 0;
//     int row = 0;
//     keyboard_state_activate();
//     while (true) {
//          char c;
//          get_keyboard_buffer(&c);
//          framebuffer_set_cursor(row, col);

//          if (c)
//          {
//             if (c == '\n')
//             {
//                 if (row != FRAMEBUFFER_HEIGHT - 1)
//                 {
//                     row++; 
//                     col = 0;
//                 }
//             }
//             else if (c == '\b')
//             {
//                 --col;
//                 if (col < 0)
//                 {
//                     col = FRAMEBUFFER_WIDTH - 1;
//                     if (row > 0) --row;
//                 }
//                 framebuffer_write(row, col, ' ', 0xF, 0);
//             }
//             else 
//             {
//                 if (!(row == FRAMEBUFFER_HEIGHT - 1 && col == FRAMEBUFFER_WIDTH))
//                     framebuffer_write(row, col++, c, 0xF, 0);
//             }

//             if (col == FRAMEBUFFER_WIDTH)
//             {
//                 if (row != FRAMEBUFFER_HEIGHT - 1)
//                 {
//                     col = 0;
//                     ++row;
//                 }
//             }
//          }
//     }

//     __asm__("int $0x4");
//     while (true);
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     activate_keyboard_interrupt();
//     initialize_idt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);

//     struct BlockBuffer b;
//     for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
//     write_blocks(&b, 17, 1);
//     while (true);
// }

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    
    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    read(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);

    while (true);
}
