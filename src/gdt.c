#include "header/cpu/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        { /* TODO: Null Descriptor */
            0
        },
        { /* TODO: Kernel Code Descriptor */
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b1010, // Code Segment, Readable, Accessed
            .non_system = 1,
            .privilege = 0b00, // Kernel Privilege Level
            .valid_bit = 1,
            .segment_high = 0xF,
            .avl = 0,
            .long_mode = 0,
            .opr_32_bit = 1, // Code Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        },
        { /* TODO: Kernel Data Descriptor */
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b0010, // Data Segment, Writable, Accessed
            .non_system = 1,
            .privilege = 0b00, // Kernel Privilege Level
            .valid_bit = 1,
            .segment_high = 0xF,
            .avl = 0,
            .long_mode = 0,
            .opr_32_bit = 1, // Data Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        },
        {/* TODO: User   Code Descriptor */
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b1010, // Code Segment, Readable, Accessed
            .non_system = 1,
            .privilege = 0x3, // Kernel Privilege Level
            .valid_bit = 1,
            .segment_high = 0xF,
            .avl = 0,
            .long_mode = 0,
            .opr_32_bit = 1, // Code Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        },
        {/* TODO: User   Data Descriptor */
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b0010, // Data Segment, Writable, Accessed
            .non_system = 1,
            .privilege = 0x3, // Kernel Privilege Level
            .valid_bit = 1,
            .segment_high = 0xF,
            .avl = 0,
            .long_mode = 0,
            .opr_32_bit = 1, // Data Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        },
        {
            .segment_high      = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .privilege         = 0,    // DPL
            .valid_bit         = 1,    // P bit
            .opr_32_bit        = 1,    // D/B bit
            .long_mode         = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    .address = &global_descriptor_table, // Alamat base diisi dengan alamat dari global_descriptor_table
    .size = sizeof(global_descriptor_table) - 1 // Ukuran limit diisi dengan ukuran dari global_descriptor_table dikurangi 1
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}