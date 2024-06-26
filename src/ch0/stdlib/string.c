#include <stdint.h>
#include <stddef.h>
#include "string.h"

void* memset(void *s, int c, size_t n) {
    uint8_t *buf = (uint8_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint8_t) c;
    return s;
}

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    uint8_t *dstbuf       = (uint8_t*) dest;
    const uint8_t *srcbuf = (const uint8_t*) src;
    for (size_t i = 0; i < n; i++)
        dstbuf[i] = srcbuf[i];
    return dstbuf;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *buf1 = (const uint8_t*) s1;
    const uint8_t *buf2 = (const uint8_t*) s2;
    for (size_t i = 0; i < n; i++) {
        if (buf1[i] < buf2[i])
            return -1;
        else if (buf1[i] > buf2[i])
            return 1;
    }

    return 0;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *dstbuf       = (uint8_t*) dest;
    const uint8_t *srcbuf = (const uint8_t*) src;
    if (dstbuf < srcbuf) {
        for (size_t i = 0; i < n; i++)
            dstbuf[i]   = srcbuf[i];
    } else {
        for (size_t i = n; i != 0; i--)
            dstbuf[i-1] = srcbuf[i-1];
    }

    return dest;
}

void clear(void *pointer, size_t n) {
    uint8_t *ptr = (uint8_t*) pointer;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = 0x00;
    }
}

size_t strlen(char *string) {
    size_t i = 0;
    while (string[i] != '\0')
        i++;
    return i;
}

uint8_t strcmp(char *s1, char *s2) {
    size_t i = 0;
    if (strlen(s1) == strlen(s2)) {
        while (s1[i] != '\0') {
            if (s1[i] != s2[i])
                return 1;
            i++;
        }
        return 0;
    }
    return 1;
}

void strcpy(char *dst, char *src, int type) {
    size_t i = 0;
    clear(dst, strlen(dst));
    if (type == 1) {
        while (src[i] != '\0') {
            dst[i] = src[i];
            i++;
        }
    } else {
        while (src[i] != 0xA) {
            dst[i] = src[i];
            i++;
        }
    }
}