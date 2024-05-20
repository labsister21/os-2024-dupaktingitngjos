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
#include "ls.h"
#include "cd.h"

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

// Find the name of directory in the dir_table and return its cluster number
int findDirectoryNumber(char* args_val, int position, int length) {
    int result = -1;

    int i = 1;
    bool found = FALSE;
    while (i < 64 && !found) {
        if (memcmp(dir_table.table[i].name, args_val + position, length) == 0 && 
            dir_table.table[i].user_attribute ==UATTR_NOT_EMPTY &&
            dir_table.table[i].attribute == ATTR_SUBDIRECTORY) {
            result = (int) ((dir_table.table[i].cluster_high << 16) | dir_table.table[i].cluster_low);
            found = TRUE;
        }
        else {
            i++;
        }
    }

    return result;
}

int findEntry(char* name) {
    int entry_index = -1;

    for (uint32_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
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
    // The buffers
    char args_val[2048];
    int args_info[128][2];
    char path_str[2048];

    // initScreen();
    syscalls (4, (uint32_t) args_val, 2048, 0x0);
    syscalls(7, 0, 0, 0);
    
    while (TRUE) {
        // Always start by clearing the buffer
        clear(args_val, 2048);
        for (int i = 0; i < 128; i++) {
            clear(args_info[i], 2);
        }
        clear(path_str, 2048);

        // Initialize
        puts("dupaktingitngjos@OS-IF2230",19, LIGHT_GREEN);
        putc(':', GREY);
        printCurrentDirectory(path_str, current_directory);
        puts("$ ", 2, GREY);
        
        // Asking for inputs
        syscalls (4, (uint32_t) args_val, 2048, 0x0);

        // Get the numbers of input args
        int args_count = inputparse (args_val, args_info);
        
        // processing the command
        if (args_count != 0) {
            if ((memcmp(args_val + *(args_info)[0], "cd", 2) == 0) && ((*(args_info))[1] == 2)) {
                cd(args_val, args_info, args_count);
            }
            else if ((memcmp(args_val + *(args_info)[0], "ls", 2) == 0) && ((*(args_info))[1] == 2)) {
                ls(args_val, args_info, args_count);
            }
            else {
                for (char i = 0; i < (*(args_info))[1]; i++) {
                    puts(args_val + (*(args_info))[0] + i, 1, RED);
                }
                puts(": command not found\n",20, RED);
            }
        }
    }

    return 0;
}
