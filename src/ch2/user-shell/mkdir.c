#include "mkdir.h"
#include "user-shell.h"
#include "../../ch0/stdlib/string.h"
#include "../../ch1/interrupt/interrupt.h"

void mkdir(char* args_value, int (*args_info)[2], int args_count) {
    // track currently visited directory, initialized to the root directory
    uint32_t search_cluster_number = ROOT_CLUSTER_NUMBER;

    // parse the arguments
    int name_first_index_position = (*(args_info + 1))[0];
    int name_length = 0;
    int index = name_first_index_position;

    int name_last_index_position = name_first_index_position + (*(args_info + 1))[1];
    bool endOfArgs1 = (name_first_index_position + name_length - 1 == name_last_index_position);
    bool endWord = TRUE;
    bool newDirectoryFound = FALSE;

    int errorCode = 0;

    // if more than 2 arguments, throw an error
    if (args_count != 2) {
        char *message = "mkdir: too many arguments\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
    }
    else {
        // if path is not absolute, set search_cluster_number to current working directory
        if (!isAbsolutePath(args_value, args_info, args_count)) {
            search_cluster_number = current_directory;
        }

        // update the directory table of the visited directory
        updateDirectoryTable(search_cluster_number);

        // start searching to make directory
        while (!endOfArgs1) {
            // If current char is not '/', process the information of word
            // else, process the word itself
            if (memcmp(args_value + index, "/", 1) != 0 && index != name_last_index_position) {
                if (!endWord) {
                    name_length++;
                }
                else {
                    // If there is a new word after non-existent directory, set an error code and stop parsing
                    if (newDirectoryFound) {
                        newDirectoryFound = FALSE;
                        errorCode = 4;
                        endOfArgs1 = TRUE;
                    }
                    else {
                        endWord = FALSE;
                        name_first_index_position = index;
                        name_length = 1;
                    }
                }
            }
            else {
                // process the word
                if (!endWord) {
                    // if word length is more than 8, set an error code and stop parsing.
                    // else, check if the word exist as directory
                    if (name_length > 8) {
                        errorCode = 3;
                        endOfArgs1 = TRUE;
                    }
                    else if (name_length == 2 && memcmp(args_value + name_first_index_position, "..", 2) == 0) {
                        search_cluster_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                        updateDirectoryTable(search_cluster_number);
                    }
                    else {
                        int temp_cluster_number = findDirectoryNumber(args_value, name_first_index_position, name_length);
                        if (temp_cluster_number == -1) {
                            newDirectoryFound = TRUE;
                        }
                        else {
                            search_cluster_number = temp_cluster_number;
                            updateDirectoryTable(search_cluster_number);
                        }
                    }
                    endWord = TRUE;
                }
            }

            if (!endOfArgs1) {
                if (index == name_last_index_position) {
                    endOfArgs1 = TRUE;
                }
                else {
                    index++;
                }
            }
        }

        if (!newDirectoryFound) {
            char *message = "mkdir: cannot create directory '";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            message = args_value + (*(args_info + 1))[0];
            syscalls(6, (uint32_t) message, (*(args_info + 1))[1], RED);
            
            switch (errorCode) {
            case 0:
                message = "': Folder exist\n";
                syscalls(6, (uint32_t) message, strlen(message), RED);
                break;
            case 3:
                message = "': Folder name is too long\n";
                syscalls(6, (uint32_t) message, strlen(message), RED);
                break;
            case 4:
                message = "': No such file or directory\n";
                syscalls(6, (uint32_t) message, strlen(message), RED);
                break;
            }
        }
        else {
            struct FAT32DriverRequest request = {
                .buf = 0,
                .name = "\0\0\0\0\0\0\0",
                .ext = "\0\0\0",
                .parent_cluster_number = search_cluster_number,
                .buffer_size = 0
            };
            memcpy(&(request.name), args_value + name_first_index_position, name_length);

            syscalls(2, (uint32_t) &request, (uint32_t) &errorCode, 0);
            if (errorCode != 0) {
                char *message = "mkdir: cannot create directory '";
                syscalls(6, (uint32_t) message, strlen(message), RED);
                message = args_value + (*(args_info + 1))[0];
                syscalls(6, (uint32_t) message, (*(args_info + 1))[1], RED);

                switch (errorCode) {
                case 1:
                    message = "': Folder exist\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    break;
                case -1:
                    message = "': Unknown error occured\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    break;
                }
            }
        }
    }
}
