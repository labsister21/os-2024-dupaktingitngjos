#ifndef __CD_H__
#define __CD_H__

#include "../../ch1/interrupt/interrupt.h"
#include "../../ch1/fat32/fat32.h"
#include "../../ch0/stdlib/string.h"

void cd(char* args_value, int (*args_info)[2], int args_count);

#endif