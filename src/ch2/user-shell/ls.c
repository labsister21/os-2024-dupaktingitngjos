#include "ls.h"
#include "user-shell.h"

void print_directory_table() {
    for (int i = 1; i < 63; i++) {
        if (dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            if (dir_table.table[i].name[7] != '\0') {
                puts(dir_table.table[i].name, 8, LIGHT_BLUE);
            }
            else {
                puts(dir_table.table[i].name, 1, LIGHT_BLUE);
            }
            if (dir_table.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[i].ext) != 0) {
                putc('.', LIGHT_BLUE);
                puts(dir_table.table[i].ext, 3, LIGHT_BLUE);
            }
            putc('\n', LIGHT_BLUE);
        }
    }

    if (dir_table.table[63].user_attribute == UATTR_NOT_EMPTY) {
        if (dir_table.table[63].name[7] != '\0') {
            puts(dir_table.table[63].name, 8, LIGHT_BLUE);
        }
        else {
            puts(dir_table.table[63].name, 1, LIGHT_BLUE);
        }
        if (dir_table.table[63].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[63].ext) != 0) {
            putc('.', LIGHT_BLUE);
            puts(dir_table.table[63].ext, 3, LIGHT_BLUE);
        }
        putc('\n', LIGHT_BLUE);
    }
}

void find_all_directory(char* args_value, int (*args_info)[2], int args_count) {
    uint32_t cluster_number = ROOT_CLUSTER_NUMBER;
    int flag = args_count;

    if (flag == -1) {
        cluster_number = current_directory;
        updateDirectoryTable(cluster_number);
        print_directory_table();
    }
    else {
        for (int i = 1; i < args_count; i++) {
            if (args_info[i][1] == 0) {
                char* path = args_value + args_info[i][0];
                if (path[0] == '/') {
                    cluster_number = ROOT_CLUSTER_NUMBER;
                    path++;
                } 
                else {
                    cluster_number = current_directory;
                }

                int j = 0;
                while (path[j] != '\0') {
                    if (path[j] == '/') {
                        j++;
                        continue;
                    }

                    int k = j;
                    while (path[k] != '/' && path[k] != '\0') {
                        k++;
                    }

                    char name[13];
                    memset(name, 0, 13);
                    memcpy(name, path + j, k - j);
                    name[k - j] = '\0';
                    
                    int entry = findEntry(name);
                    if (entry == -1) {
                        puts("ls: no such file or directory\n", 30, RED);
                        return;
                    }

                    if (dir_table.table[entry].attribute & ATTR_SUBDIRECTORY) {
                        cluster_number = dir_table.table[entry].cluster_low;
                    } 
                    else {
                        puts("ls: not a directory\n", 21, RED);
                        return;
                    }
                    j = k;
                }
                updateDirectoryTable(cluster_number);
                print_directory_table();
            }
        }
    }
}

void ls(char* args_value, int (*args_info)[2], int args_count) {
    if (args_count == 1) {
        find_all_directory(args_value, args_info, -1);
    }
    else {
        find_all_directory(args_value, args_info, args_count);
    }
}