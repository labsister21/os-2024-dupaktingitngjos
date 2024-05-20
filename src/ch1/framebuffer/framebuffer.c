#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "framebuffer.h"
#include "../../ch0/stdlib/string.h"
#include "../portio/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
        // TODO : Implement
        uint16_t position = r * FRAMEBUFFER_WIDTH + c;

        // Set low byte of the cursor location
        out(CURSOR_PORT_CMD, 0x0F);
        out(CURSOR_PORT_DATA, (uint8_t)(position & 0xFF));

        // Set high byte of the cursor location
        out(CURSOR_PORT_CMD, 0x0E);
        out(CURSOR_PORT_DATA, (uint8_t)((position >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
        // TODO : Implement
        if (row >= FRAMEBUFFER_HEIGHT || col >= FRAMEBUFFER_WIDTH)
                return;

        uint16_t index = (row * FRAMEBUFFER_WIDTH + col) * 2;
        FRAMEBUFFER_MEMORY_OFFSET[index] = c;
        FRAMEBUFFER_MEMORY_OFFSET[index + 1] = ((bg << 4) & 0xF0) | (fg & 0x0F);
}

void framebuffer_clear(void) {
        // TODO : Implement
        for (int i = 0; i < FRAMEBUFFER_HEIGHT; i++) {
                for (int j = 0; j < FRAMEBUFFER_WIDTH; j++) {
                        framebuffer_write(i, j, ' ', 0x07, 0x00);
                }
        }
}