#include "header/cpu/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        { // Null
            0
        },
        { // Kernel Code Segment
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b1010, // Code Segment, Readable, Accessed
            .non_system = 1,
            .dpl = 0b00, // Kernel Privilege Level
            .present = 1,
            .limit_high = 0xF,
            .avl = 0,
            .reserved = 0,
            .operation = 1, // Code Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        },
        { // Kernel Data Segment
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b0010, // Data Segment, Writable, Accessed
            .non_system = 1,
            .dpl = 0b00, // Kernel Privilege Level
            .present = 1,
            .limit_high = 0xF,
            .avl = 0,
            .reserved = 0,
            .operation = 1, // Data Segment
            .granularity = 1, // 4KB granularity
            .base_high = 0
        }
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
