#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

// Static driver state
static struct FAT32DriverState driver_state;





/* -- Driver Interfaces -- */

uint32_t cluster_to_lba(uint32_t cluster) {
    // 1 cluster = 4 blocks
    return cluster * CLUSTER_BLOCK_COUNT;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
    // Clear directory table
    for (uint32_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); ++i) {
        memset(&dir_table->table[i], 0, sizeof(struct FAT32DirectoryEntry));
    }
    
    struct FAT32DirectoryEntry *entry;

    // Entry-0: DirectoryEntry about itself
    entry = &dir_table->table[0];
    memcpy(entry->name, name, 8);
    entry->attribute = ATTR_SUBDIRECTORY;
    entry->user_attribute = UATTR_NOT_EMPTY;
    // self directory cluster number not set yet

    // Entry-1: Parent DirectoryEntry
    entry = &dir_table->table[1];
    memcpy(entry->name, "..", 2);
    entry->attribute = ATTR_SUBDIRECTORY;
    entry->user_attribute = UATTR_NOT_EMPTY;
    entry->cluster_high = parent_dir_cluster >> 16;
    entry->cluster_low = parent_dir_cluster;
}

bool is_empty_storage(void) {
    struct BlockBuffer boot_sector;
    read_blocks(boot_sector.buf, BOOT_SECTOR, 1);
    return memcmp(boot_sector.buf, fs_signature, BLOCK_SIZE) != 0;
}

void create_fat32(void) {
    // Write fs_signature into boot sector
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    // Initialize and write FAT into cluster number 1
    struct FAT32FileAllocationTable file_allocation_table = {
        .cluster_map = {
            [0] = CLUSTER_0_VALUE,
            [FAT_CLUSTER_NUMBER] = CLUSTER_1_VALUE,
            [ROOT_CLUSTER_NUMBER] = FAT32_FAT_END_OF_FILE,
        },
    };
    write_clusters(file_allocation_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // Initialize and write "root" directory table into cluster number 2
    struct FAT32DirectoryTable root_directory_table = {
        .table = {
            [0] = {
                .name = "root",
                .ext = "",
                .attribute = ATTR_SUBDIRECTORY,
                .user_attribute = UATTR_NOT_EMPTY,
                .cluster_high = 0,
                .cluster_low = ROOT_CLUSTER_NUMBER,
                .filesize = 0,
            },
            [1] = {
                .name = "..",
                .ext = "",
                .attribute = ATTR_SUBDIRECTORY,
                .user_attribute = UATTR_NOT_EMPTY,
                .cluster_high = 0,
                .cluster_low = ROOT_CLUSTER_NUMBER,
                .filesize = 0,
            },
        },
    };
    write_clusters(root_directory_table.table, ROOT_CLUSTER_NUMBER, 1);
}

void initialize_filesystem_fat32(void) {
    if (is_empty_storage()) {
        create_fat32();
    }
    
    // Cache FAT and "root" directory table into driver state
    read_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    read_clusters(driver_state.dir_table_buf.table, ROOT_CLUSTER_NUMBER, 1);
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    if (cluster_count > 63) {
        return;
    }
    
    uint8_t *ptr2 = (uint8_t *) ptr;
    struct ClusterBuffer cluster_buffer;
    for (uint8_t i = 0; i < cluster_count; ++i) {
        memcpy(cluster_buffer.buf, ptr2 + i * CLUSTER_SIZE, CLUSTER_SIZE);  
        write_blocks(cluster_buffer.buf, cluster_to_lba(cluster_number + i), CLUSTER_BLOCK_COUNT);
    }
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    if (cluster_count > 63) {
        return;
    }
    
    uint8_t *ptr2 = (uint8_t *) ptr;
    struct ClusterBuffer cluster_buffer;
    for (uint8_t i = 0; i < cluster_count; ++i) {
        read_blocks(cluster_buffer.buf, cluster_to_lba(cluster_number + i), CLUSTER_BLOCK_COUNT);
        memcpy(ptr2 + i * CLUSTER_SIZE, cluster_buffer.buf, CLUSTER_SIZE);
    }
}





/* -- CRUD Operation -- */

int8_t read_directory(struct FAT32DriverRequest request) {
    // Check reserved cluster number
    if (request.parent_cluster_number < 2) {
        return -1;
    }

    // Check buffer size
    if (request.buffer_size != sizeof(struct FAT32DirectoryTable)) {
        return -1;
    }

    // Check FAT for parent directory
    if (driver_state.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE) {
        return -1;
    }

    // Load directory table from parent cluster
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    
    // Check validity of the directory table
    if (driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    // Search for directory name
    char identifier_request[11];
    memset(identifier_request, 0, 11);
    memcpy(identifier_request, request.name, 8);
    char identifier_search[11];
    for (uint32_t i = 2; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); ++i) {
        memcpy(identifier_search, driver_state.dir_table_buf.table[i].name, 8);
        memcpy(identifier_search + 8, driver_state.dir_table_buf.table[i].ext, 3);
        if (driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY &&
            memcmp(identifier_request, identifier_search, 11) == 0) {
            if (driver_state.dir_table_buf.table[i].attribute != ATTR_SUBDIRECTORY) {
                // Directory name is found but not a directory
                return 1;
            }
            // Directory name is found and is a directory
            uint32_t cluster_number = 
                driver_state.dir_table_buf.table[i].cluster_low |
                (driver_state.dir_table_buf.table[i].cluster_high << 16);
            read_clusters(request.buf, cluster_number, 1);
            return 0;
        }
    }

    return 2;
}

int8_t read(struct FAT32DriverRequest request) {
    // Check reserved cluster number
    if (request.parent_cluster_number < 2) {
        return -1;
    }
    
    // Check FAT for parent directory
    if (driver_state.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE) {
        return -1;
    }

    // Load directory table from parent cluster
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    
    // Check validity of the directory table
    if (driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    // Search for file name
    char identifier_request[11];
    memcpy(identifier_request, request.name, 8);
    memcpy(identifier_request + 8, request.ext, 3);
    char identifier_search[11];
    for (uint32_t i = 2; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); ++i) {
        memcpy(identifier_search, driver_state.dir_table_buf.table[i].name, 8);
        memcpy(identifier_search + 8, driver_state.dir_table_buf.table[i].ext, 3);
        if (driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY &&
            memcmp(identifier_request, identifier_search, 11) == 0) {
            if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY) {
                // File name is found but not a file
                return 1;
            }
            // File name is found and is a file

            // Check buffer size
            if (request.buffer_size < driver_state.dir_table_buf.table[i].filesize) {
                return 2;
            }

            uint32_t cluster_number = 
                driver_state.dir_table_buf.table[i].cluster_low |
                (driver_state.dir_table_buf.table[i].cluster_high << 16);
            uint8_t *temp_buf = (uint8_t *) request.buf;
            // Read until end of file
            do {
                read_clusters(temp_buf, cluster_number, 1);
                temp_buf += CLUSTER_SIZE;
                cluster_number = driver_state.fat_table.cluster_map[cluster_number];
            } while (cluster_number != FAT32_FAT_END_OF_FILE);

            request.buffer_size = driver_state.dir_table_buf.table[i].filesize;
            return 0;
        }
    }

    return 3;
}

int8_t write(struct FAT32DriverRequest request) {
    // Check reserved cluster number
    if (request.parent_cluster_number < 2) {
        return 2;
    }

    // Check FAT for parent directory
    if (driver_state.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE) {
        return 2;
    }

    // Load directory table from parent cluster
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    // Check validity of the directory table
    if (driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY) {
        return 2;
    }

    // Check if file already exist
    char identifier_request[11];
    memcpy(identifier_request, request.name, 8);
    memcpy(identifier_request + 8, request.ext, 3);
    char identifier_search[11];
    for (uint32_t i = 2; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); ++i) {
        memcpy(identifier_search, driver_state.dir_table_buf.table[i].name, 8);
        memcpy(identifier_search + 8, driver_state.dir_table_buf.table[i].ext, 3);
        if (driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY &&
            memcmp(identifier_request, identifier_search, 11) == 0) {
            return 1;
        }
    }

    // Check for an empty parent directory table entry
    bool directory_table_full = true;
    uint32_t directory_table_index = 2;
    while (directory_table_index < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && 
           directory_table_full) {
        if (driver_state.dir_table_buf.table[directory_table_index].user_attribute != UATTR_NOT_EMPTY) {
            directory_table_full = false;
        } else {
            ++directory_table_index;
        }
    }

    if (directory_table_full) {
        return -1;
    }

    // Check for available FAT entry
    uint32_t count_available = 0;
    for (int i = 2; i < CLUSTER_MAP_SIZE; ++i) {
        if (driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY) {
            ++count_available;
        }
    }

    if (count_available < request.buffer_size / CLUSTER_SIZE + 1) {
        return -1;
    }

    // Add entry to parent directory table (cluster and filesize will be added later)
    memcpy(driver_state.dir_table_buf.table[directory_table_index].name, request.name, 8);
    memcpy(driver_state.dir_table_buf.table[directory_table_index].ext, request.ext, 3);
    if (request.buffer_size == 0) {
        driver_state.dir_table_buf.table[directory_table_index].attribute = ATTR_SUBDIRECTORY;
    }

    driver_state.dir_table_buf.table[directory_table_index].user_attribute = UATTR_NOT_EMPTY;
    
    // Write the buffer of the driver request
    uint8_t *temp_buf = (uint8_t *) request.buf;
    uint32_t fat_index = 2;
    uint32_t previous_fat_index = 0;
    uint32_t i = 0;
    do {
        // Check for first empty FAT entry
        while (driver_state.fat_table.cluster_map[i] != FAT32_FAT_EMPTY_ENTRY) {
            ++fat_index;
        }

        if (i == 0) {
            // Add cluster index into the new parent directory table entry
            driver_state.dir_table_buf.table[directory_table_index].cluster_low = fat_index;
            driver_state.dir_table_buf.table[directory_table_index].cluster_high = fat_index >> 16;
        } else {
            // Add cluster index into the FAT entry
            driver_state.fat_table.cluster_map[previous_fat_index] = fat_index;
        }

        previous_fat_index = fat_index;
        write_clusters(temp_buf + i * CLUSTER_SIZE, fat_index, 1);
        ++i;
    } while (i < request.buffer_size / CLUSTER_SIZE + 1);

    // Add filesize into the new parent directory table entry
    driver_state.dir_table_buf.table[directory_table_index].filesize = i * CLUSTER_SIZE;
    
    // Add end of file marker into the last FAT entry
    driver_state.fat_table.cluster_map[fat_index] = FAT32_FAT_END_OF_FILE;

    // Write FAT and parent directory table into non-volatile memory
    write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    return 0;
}
    
int8_t delete(struct FAT32DriverRequest request) {
    // Check reserved cluster number
    if (request.parent_cluster_number < 2) {
        return -1;
    }

    // Check FAT for parent directory
    if (driver_state.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE) {
        return -1;
    }

    // Load directory table from parent cluster
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    
    // Check validity of the directory table
    if (driver_state.dir_table_buf.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    // Search for file/folder name
    char identifier_request[11];
    memcpy(identifier_request, request.name, 8);
    memcpy(identifier_request + 8, request.ext, 3);
    char identifier_search[11];
    
    bool found = false;
    uint32_t directory_table_index = 2;
    while (directory_table_index < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) && 
           !found) {
        memcpy(identifier_search, driver_state.dir_table_buf.table[directory_table_index].name, 8);
        memcpy(identifier_search + 8, driver_state.dir_table_buf.table[directory_table_index].ext, 3);
        if (driver_state.dir_table_buf.table[directory_table_index].user_attribute == UATTR_NOT_EMPTY &&
            memcmp(identifier_request, identifier_search, 11) == 0) {
            found = true;
        } else {
            ++directory_table_index;
        }
    }

    if (!found) {
        return 1;
    }

    // Check if the entry is a directory
    bool is_directory = driver_state.dir_table_buf.table[directory_table_index].attribute == ATTR_SUBDIRECTORY;
    if (is_directory) {
        // Check if the directory is empty
        uint32_t cluster_number = 
            driver_state.dir_table_buf.table[directory_table_index].cluster_low |
            (driver_state.dir_table_buf.table[directory_table_index].cluster_high << 16);
        read_clusters(driver_state.dir_table_buf.table, cluster_number, 1);
        for (uint32_t i = 2; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); ++i) {
            if (driver_state.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY) {
                return 2;
            }
        }
    }

    // Delete the FAT entries
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    uint32_t cluster_number = 
        driver_state.dir_table_buf.table[directory_table_index].cluster_low |
        (driver_state.dir_table_buf.table[directory_table_index].cluster_high << 16);
    uint32_t current_fat_index = cluster_number;
    uint32_t next_fat_index = driver_state.fat_table.cluster_map[current_fat_index];
    while (next_fat_index != FAT32_FAT_END_OF_FILE) {
        driver_state.fat_table.cluster_map[current_fat_index] = FAT32_FAT_EMPTY_ENTRY;
        current_fat_index = next_fat_index;
        next_fat_index = driver_state.fat_table.cluster_map[current_fat_index];
    }

    driver_state.fat_table.cluster_map[current_fat_index] = FAT32_FAT_EMPTY_ENTRY;

    // Delete the parent directory table entry
    driver_state.dir_table_buf.table[directory_table_index].user_attribute = 0;

    // Write FAT and parent directory table into non-volatile memory
    write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    return 0;
}
