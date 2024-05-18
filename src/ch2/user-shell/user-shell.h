#ifndef __USER_SHELL_H
#define __USER_SHELL_H

#include <stdint.h>
#include "../../ch1/fat32/fat32.h"

// Color
#define GREY         0b0111
#define RED          0b1100
#define DARK_GREY    0b1000
#define LIGHT_BLUE   0b1001
#define LIGHT_GREEN  0b1010
#define BROWN        0b0110
#define WHITE        0b1111
#define BLACK        0b0000

// Boolean
#define bool int
#define FALSE 0
#define TRUE 1

// Filesystem
uint32_t current_directory = ROOT_CLUSTER_NUMBER;
struct FAT32DirectoryTable dir_table;

// syscall to main
void syscalls(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// parse input
int inputparse (char *args_val, int args_info[128][2]);

// Print Current Directory
void printCurrentDirectory (char* path, uint32_t current_dir);

/* Functions for pathing */
bool isAbsolutePath(char* args_val, int (*args_info)[2], int args_position);

void updateDirectoryTable(uint32_t cluster_number);

int findEntry(char* name);

/* Splash screen */
void screenInit();

#endif