#include "cd.h"
#include "user-shell.h"

void cd(char* args_value, int (*args_info)[2], int args_count) {
    if (args_count == 2) {
        uint32_t cluster_number = ROOT_CLUSTER_NUMBER;
        char* path = args_value + args_info[1][0];

        if (path[0] == '/') {
            cluster_number = ROOT_CLUSTER_NUMBER;
            path++;
        } 
        else {
            cluster_number = current_directory;
        }

        int i = 0;
        while (path[i] != '\0') {
            if (path[i] == '/') {
                i++;
                continue;
            }

            int j = i;
            while (path[j] != '/' && path[j] != '\0') {
                j++;
            }

            char name[13];
            memset(name, 0, 13);
            memcpy(name, path + i, j - i);
            name[j - i] = '\0';
            
            int entry = findEntry(name);
            if (entry == -1) {
                puts("cd: no such file or directory\n", 30, RED);
                return;
            }

            if (dir_table.table[entry].attribute & ATTR_SUBDIRECTORY) {
                cluster_number = dir_table.table[entry].cluster_low;
            } 
            else {
                puts("cd: not a directory\n", 21, RED);
                return;
            }
            i = j;
        }

        current_directory = cluster_number;
        updateDirectoryTable(cluster_number);
    } 
    else if (args_count > 2) {
        puts("cd: too many arguments\n", 23, RED); 
    } 
    else {
        puts("cd: missing argument\n", 21, RED);
    }
}

