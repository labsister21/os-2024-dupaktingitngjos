#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.read_write        = 1,
            .flag.user_supervisor   = 1,
            .flag.write_through     = 1,
            .flag.cache_disabled    = 1,
            .flag.accessed          = 1,
            .flag.dirty             = 1,
            .flag.page_size         = 1,

            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.read_write        = 1,
            .flag.user_supervisor   = 1,
            .flag.write_through     = 1,
            .flag.cache_disabled    = 1,
            .flag.accessed          = 1,
            .flag.dirty             = 1,
            .flag.page_size         = 1,
            
            .lower_address          = 0,
        },
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false
    },

    // TODO: Initialize page manager state properly
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT,
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    return (amount <= page_manager_state.free_page_frame_count);
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */ 

    uint32_t physical_idx;
    for (physical_idx = 0; physical_idx < PAGE_FRAME_MAX_COUNT; ++physical_idx)
        if (!page_manager_state.page_frame_map[physical_idx]) 
        {
            page_manager_state.page_frame_map[physical_idx] = true;
            --page_manager_state.free_page_frame_count;
            break;
        }

    //  Ga ketemu frame yang free
    if (physical_idx >= PAGE_FRAME_MAX_COUNT) 
        return false;

    uint32_t page_idx = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_idx].flag.present_bit     = false;
    page_dir->table[page_idx].flag.read_write      = false;
    page_dir->table[page_idx].flag.user_supervisor = false;
    page_dir->table[page_idx].flag.page_size       = false;

    return true;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */

    uint32_t page_idx = ((uint32_t) virtual_addr >> 22) & 0x3FF;

    // Entri ga ada
    if (!page_dir->table[page_idx].flag.present_bit)
        return false;

    uint32_t physical_idx = page_dir->table[page_idx].lower_address;
    page_manager_state.page_frame_map[physical_idx] = false;
    ++page_manager_state.free_page_frame_count;

    page_dir->table[page_idx].flag.present_bit     = 0;
    page_dir->table[page_idx].flag.read_write      = 0;
    page_dir->table[page_idx].flag.user_supervisor = 0;
    page_dir->table[page_idx].flag.page_size       = 0;

    return true;
}