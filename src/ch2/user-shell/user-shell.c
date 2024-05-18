// int main(void) {
//     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
//     return 0;
// }

#include <stdint.h>
#include "../../ch1/fat32/fat32.h"
#include "../../ch1/interrupt/interrupt.h"
#include "../../ch0/stdlib/string.h"
#include "user-shell.h"
#include "user-shell.h"

void syscalls(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// parse input
int inputparse (char *args_val, int args_info[128][2]) {
    // Declare the vars
    int nums = 0;

    // iterating vars
    int i = 0; // for each char
    int j = 0; // for each word
    int k = 0; // for each char in word

    // Flags
    bool endWord = TRUE;
    bool startWord = TRUE;
    int countChar = 0;

    // Iterate all chars
    // Ignore blanks
    while (args_val[i] == ' ' && args_val[i] != 0x0A) {
        i++;
    }

    // While not end of line
    while (args_val[i] != 0x0A) {
        // Ignore blanks
        while (args_val[i] == ' ' && args_val[i] != 0x0A) {
            if (!endWord) {
                k = 0;
                j++;
                endWord = TRUE;
            }
            startWord = TRUE;
            i++;
        }

        // Return the number of args
        if (args_val[i] == 0x0A) {
            return nums;
        }

        // Out then it is not the end of the word
        endWord = FALSE;

        // Process other chars
        if (startWord) {
            nums++;
            countChar = 0;
            args_info[j][k] = i;
            startWord = FALSE;
            k++;
        }

        countChar++;
        args_info[j][k] = countChar;
        i++; // Next char
    }

    return nums;
}

// Print Current Directory
void printCurrentDirectory (char* path, uint32_t current_dir) {
    // init vars
    int path_len = 0;
    int node_count = 0;
    char node_index[10][64];

    // hapus path
    clear(path, 128);
    for (int i = 0; i < 10; i++) {
        clear(node_index[i], 64);
    }

    if (current_dir == ROOT_CLUSTER_NUMBER) {
        path_len++;
        path[path_len] = '/';
        puts(path, path_len, LIGHT_BLUE);
        return;
    }

    // loop to get the path
    uint32_t cluster_number = current_dir;
    path_len++;
    path[path_len] = '/';
    while (cluster_number != ROOT_CLUSTER_NUMBER) {
        updateDirectoryTable(cluster_number);
        memcpy(node_index[node_count], dir_table.table[0].name, strlen(dir_table.table[0].name));
        node_count++;
        cluster_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
    }

    // iterate the path
    for (int i = node_count - 1; i >= 0; i--) {
        int len = strlen(node_index[i]);
        for (int j = 0; j < len; j++) {
            path_len++;
            path[path_len] = node_index[i][j];
        }
        path_len++;
        path[path_len] = '/';
    }

    // print the path
    puts(path, path_len, LIGHT_BLUE);
}

/* Functions for pathing */
bool isAbsolutePath(char* args_val, int (*args_info)[2], int args_position) {
    return (memcmp(args_val + (*(args_info + args_position))[0], "/", 1) == 0);
}

void updateDirectoryTable(uint32_t cluster_number) {
    syscalls(6, (uint32_t) &dir_table, cluster_number, 0x0);
}

int findEntry(char* name) {
    int entry_index = -1;

    for (int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        if (memcmp(dir_table.table[i].name, name, 8) == 0) {
            entry_index = i;
            break;
        }
    }
    
    return entry_index;
}

/* Splash screen */
void screenInit();

int main(void) {
    struct ClusterBuffer      cl[2]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int32_t retcode;
    syscalls(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        syscalls(6, (uint32_t) "owo\n", 4, 0xF);

    char buf;
    syscalls(7, 0, 0, 0);
    while (true) {
        syscalls(4, (uint32_t) &buf, 0, 0);
        syscalls(5, (uint32_t) &buf, 0xF, 0);
    }

    return 0;
}
