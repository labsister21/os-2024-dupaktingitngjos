#ifndef __LS_H__
#define __LS_H__

#include "../../ch1/interrupt/interrupt.h"
#include "../../ch1/fat32/fat32.h"
#include "../../ch0/stdlib/string.h"

void print_directory_table();

void find_all_directory(char* args_value, int (*args_info)[2], int args_count);

void ls(char* args_value, int (*args_info)[2], int args_count);

#endif