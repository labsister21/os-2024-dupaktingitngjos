#include "cp.h"
#include "user-shell.h"
#include "../../ch0/stdlib/string.h"

void copy(char* args_value, int (*args_info)[2], int args_count) {
    /* Searches if the destination exists and if it is a file or directory.
       Returns 1 if it is a file, 0 if it is a directory, -1 if it is not found */

    // Variables to keep track the currently visited directory
    uint32_t dest_search_directory_number = ROOT_CLUSTER_NUMBER;
    char destName[8];
    memset(destName, 0, sizeof(char) * 8);
    char destExt[3];
    memset(destName, 0, sizeof(char) * 3);

    // Variables for parsing the arguments
    int posName = (*(args_info + args_count-1))[0];
    int lenName = 0;
    int index = posName;
    int entry_index = -1;

    int posEndArgs = (*(args_info + args_count-1))[0] + (*(args_info + args_count-1))[1];
    bool endOfArgs = (posName+lenName-1 == posEndArgs);
    bool endWord = TRUE;
    bool newFileFound = FALSE;

    int errorCode = 0;

    // If path is not absolute, set the currently visited directory to current working directory
    if (!isAbsolutePath(args_value, args_info, args_count-1)) {
        dest_search_directory_number = current_directory;
    }

    // Get the directory table of the visited directory
    updateDirectoryTable(dest_search_directory_number);

    // Start searching for the directory to make 
    while (!endOfArgs) {
        // If current char is not '/', process the information of word. Else, process the word itself
        if (memcmp(args_value + index, "/", 1) != 0 && index != posEndArgs) {
            // If word already started, increment the length. Else, start new word
            if (!endWord) {
                lenName++;
            }
            else {
                // If there is a new word after non-existent directory, set an error code and stop parsing
                if (newFileFound) {
                    newFileFound = FALSE;
                    if (errorCode == 5) {
                        errorCode = 1;
                    }
                    else {
                        errorCode = 4;
                    }
                    endOfArgs = TRUE;
                }
                else {
                    endWord = FALSE;
                    posName = index;
                    lenName = 1;
                }
            }
        }
        else {
            // Process the word
            if (!endWord) {
                // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
                if (lenName > 8) {
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_value + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i >= lenName) {
                        errorCode = 3;
                        endOfArgs = TRUE;
                    } else if (lenName-i-1 > 3) {
                        errorCode = 3;
                        endOfArgs = TRUE;
                    } else {
                        clear(destName, 8);
                        clear(destExt,3);
                        memcpy(destName, args_value + posName, i);
                        if (*(args_value + posName + i + 1) != 0x0A) {
                            memcpy(destExt, args_value + posName + i + 1, lenName-i-1);
                        }
                        entry_index = findEntry(destName);
                        if (entry_index == -1) {
                            newFileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(dest_search_directory_number);
                            }
                            else {
                                newFileFound = TRUE;
                                errorCode = 5;
                            }
                        }
                    }
                    endWord = TRUE;
                }
                else if (lenName == 2 && memcmp(args_value + posName, "..", 2) == 0) {
                    dest_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                    updateDirectoryTable(dest_search_directory_number);
                }
                else {
                    clear(destName, 8);
                    clear(destExt,3);
                    // Cek extension
                    int i = 0;
                    while (i < lenName && memcmp(".", args_value + posName + i, 1) != 0) {
                        i++;
                    }
                    if (i < lenName) { // Jika ada extension
                        if (lenName-i-1 > 3) { // Jika extension lebih dari 3 karakter
                            errorCode = 3;
                            break;
                        }   
                        memcpy(destName, args_value + posName, i);
                        if (*(args_value + posName + i + 1) != 0x0A) {
                            memcpy(destExt, args_value + posName + i + 1, lenName-i-1);
                        }
                    } else {
                        memcpy(destName, args_value + posName, lenName);
                    }
                    entry_index = findEntry(destName);
                    if (entry_index == -1) {
                        newFileFound = TRUE;
                    }
                    else {
                        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                            dest_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                            updateDirectoryTable(dest_search_directory_number);
                        }
                        else {
                            newFileFound = TRUE;
                            errorCode = 5;
                        }
                    }
                }
                endWord = TRUE;
            }
        }

        if (!endOfArgs) {
            if (index == posEndArgs) {
                endOfArgs = TRUE;
            }
            else {
                index++;
            }
        }
    }

    if (errorCode == 3 || errorCode ==  4) {
        char *message = "cp: Destination not valid\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }
    else if (args_count > 3 && newFileFound) {
        // If more than 1 file copied, it must be put into a folder
        char *message = "cp: target '";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        message = args_value + (*(args_info + args_count-1))[0];
        syscalls(6, (uint32_t) message, (*(args_info + args_count-1))[1], RED);
        message = "' is not a directory\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }

    // Read each files that need to be copied
    for (int j=1; j<args_count-1; j++) {
        // Variables to keep track the currently visited directory
        uint32_t src_search_directory_number = ROOT_CLUSTER_NUMBER;
        char srcName[8];
        memset(srcName, 0, sizeof(char) * 8);
        char srcExt[3];
        memset(srcExt, 0, sizeof(char) * 3);

        // Variables for parsing the arguments
        posName = (*(args_info + j))[0];
        lenName = 0;
        index = posName;
        entry_index = -1;

        posEndArgs = (*(args_info + j))[0] + (*(args_info + j))[1];
        endOfArgs = (posName + lenName - 1 == posEndArgs);
        endWord = TRUE;
        bool srcNewFileFound = FALSE;

        errorCode = 0;

        // If path is not absolute, set the currently visited directory to current working directory
        if (!isAbsolutePath(args_value, args_info, j)) {
            src_search_directory_number = current_directory;
        }

        // Get the directory table of the visited directory
        updateDirectoryTable(src_search_directory_number);

        // Start searching for the directory to make 
        while (!endOfArgs) {
            // If current char is not '/', process the information of word. Else, process the word itself
            if (memcmp(args_value + index, "/", 1) != 0 && index != posEndArgs) {
                // If word already started, increment the length. Else, start new word
                if (!endWord) {
                    lenName++;
                }
                else {
                    // If there is a new word after non-existent directory, set an error code and stop parsing
                    if (srcNewFileFound) {
                        srcNewFileFound = FALSE;
                        if (errorCode == 5) {
                            errorCode = 1;
                        }
                        else {
                            errorCode = 4;
                        }
                        endOfArgs = TRUE;
                    }
                    else {
                        endWord = FALSE;
                        posName = index;
                        lenName = 1;
                    }
                }
            }
            else {
                // Process the word
                if (!endWord) {
                    // If word length more than 8, set an error code and stop parsing. Else, check if the word exist as directory
                    if (lenName > 8) {
                        // Periksa extension
                        int i = 0;
                        while (i < lenName && memcmp(".", args_value + posName + i, 1) != 0) {
                            i++;
                        }
                        clear(srcName, 8);
                        clear(srcExt,3);
                        if (i >= lenName) {
                            errorCode = 3;
                            endOfArgs = TRUE;
                        } 
                        else if (lenName-i-1 > 3) {
                            errorCode = 3;
                            endOfArgs = TRUE;
                        } 
                        else {
                            memcpy(srcName, args_value + posName, i);
                            if (*(args_value + posName + i + 1) != 0x0A) {
                                memcpy(srcExt, args_value + posName + i + 1, lenName-i-1);
                            }
                            entry_index = findEntry(srcName);
                            if (entry_index == -1) {
                                srcNewFileFound = TRUE;
                            }
                            else {
                                if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                    src_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                    updateDirectoryTable(src_search_directory_number);
                                }
                                else {
                                    srcNewFileFound = TRUE;
                                    errorCode = 5;
                                }
                            }
                        }
                        endWord = TRUE;
                    }
                    else if (lenName == 2 && memcmp(args_value + posName, "..", 2) == 0) {
                        src_search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
                        updateDirectoryTable(src_search_directory_number);
                    }
                    else {
                        clear(srcName, 8);
                        clear(srcExt,3);
                        // Periksa extension
                        int i = 0;
                        while (i < lenName && memcmp(".", args_value + posName + i, 1) != 0) {
                            i++;
                        }
                        if (i < lenName) { // Jika ada extension
                            if (lenName-i-1 > 3) { // Jika extension lebih dari 3 karakter
                                errorCode = 3;
                                break;
                            }
                            memcpy(srcName, args_value + posName, i);
                            if (*(args_value + posName + i + 1) != 0x0A) {
                                memcpy(srcExt, args_value + posName + i + 1, lenName-i-1);
                            }
                        } else {
                            memcpy(srcName, args_value + posName, lenName);
                        }
                        entry_index = findEntry(srcName);
                        if (entry_index == -1) {
                            srcNewFileFound = TRUE;
                        }
                        else {
                            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                                src_search_directory_number = (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);
                                updateDirectoryTable(src_search_directory_number);
                            }
                            else {
                                srcNewFileFound = TRUE;
                                errorCode = 5;
                            }
                        }
                    }
                    endWord = TRUE;
                }
            }

            if (!endOfArgs) {
                if (index == posEndArgs) {
                    endOfArgs = TRUE;
                }
                else {
                    index++;
                }
            }
        }

        if (errorCode == 3 || errorCode ==  4) {
            char *message = "cp: cannot stat '";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            message = args_value + (*(args_info + j))[0];
            syscalls(6, (uint32_t) message, (*(args_info + j))[1], RED);
            message = "': No such file or directory\n";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            return;
        }
        else if (!srcNewFileFound) {
            char *message = "cp: '";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            message = args_value + (*(args_info + j))[0];
            syscalls(6, (uint32_t) message, (*(args_info + j))[1], RED);
            message = "' is a directory\n";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            return;
        }

        struct ClusterBuffer clusterBuffer;
        memset(&clusterBuffer, 0, sizeof(struct ClusterBuffer));
        struct FAT32DriverRequest srcRequest = {
            .buf = &clusterBuffer,
            .name = "\0\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = src_search_directory_number,
            .buffer_size = CLUSTER_SIZE
        };
        memcpy(&(srcRequest.name), srcName, 8);
        memcpy(&(srcRequest.ext), srcExt, 3);
        int retCode;

        syscalls(0, (uint32_t) &srcRequest, (uint32_t) &retCode, 0x0);

        if (retCode != 0) {
            char *message = "cp: cannot start '";
            syscalls(6, (uint32_t) message, strlen(message), RED);
            message = args_value + (*(args_info + j))[0];
            syscalls(6, (uint32_t) message, (*(args_info + j))[1], RED);
            switch (retCode) {
                case 1:
                    message = "': Is a directory\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    return;
                case 2:
                    message = "': Buffer size is not enough\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    return;
                case 3:
                    message = "': No such file or directory\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    return;
                case -1:
                    message = "': Unknown error\n";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    return;
            }
        } else {
            if (!newFileFound) {
                // Tujuan berupa direktori
                srcRequest.parent_cluster_number = dest_search_directory_number;
                syscalls(3, (uint32_t) &srcRequest, (uint32_t) &retCode, 0x0);
                syscalls(2, (uint32_t) &srcRequest, (uint32_t) &retCode, 0x0);
                if (retCode != 0) {
                    char *message = "cp: cannot copy '";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    message = args_value + (*(args_info + j))[0];
                    syscalls(6, (uint32_t) message, (*(args_info + j))[1], RED);
                    switch (retCode) {
                    case 1:
                        message = "': File exist\n";
                        syscalls(6, (uint32_t) message, strlen(message), RED);
                        return;
                    case -1:
                        message = "': Unknown error occured\n";
                        syscalls(6, (uint32_t) message, strlen(message), RED);
                        return;
                    }
                }
            } else {
                // Tujuan berupa file
                struct FAT32DriverRequest destReq = {
                    .buf = &clusterBuffer,
                    .name = "\0\0\0\0\0\0\0\0",
                    .ext = "\0\0\0",
                    .parent_cluster_number = dest_search_directory_number,
                    .buffer_size = CLUSTER_SIZE
                };
                memcpy(&destReq.name, destName, 8);
                memcpy(&destReq.ext, destExt, 3);
                syscalls(3, (uint32_t) &destReq, (uint32_t) &retCode, 0);
                syscalls(2, (uint32_t) &destReq, (uint32_t) &retCode, 0);
                if (retCode != 0) {
                    char *message = "cp: cannot copy '";
                    syscalls(6, (uint32_t) message, strlen(message), RED);
                    message = args_value + (*(args_info + j))[0];
                    syscalls(6, (uint32_t) message, (*(args_info + j))[1], RED);
                    switch (retCode) {
                    case 1:
                        message = "': File exist\n";
                        syscalls(6, (uint32_t) message, strlen(message), RED);
                        return;
                    case -1:
                        message = "': Unknown error occured\n";
                        syscalls(6, (uint32_t) message, strlen(message), RED);
                        return;
                    }
                }
            }
        }
    }
}

void cp(char* args_value, int (*args_info)[2], int args_count) {
    if (args_count > 2) {
        copy(args_value, args_info, args_count);
    }
    else if (args_count == 2) {
        char *message = "cp: missing destination file operand after '";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        message = args_value + (*(args_info + 1))[0];
        syscalls(6, (uint32_t) message, (*(args_info + 1))[1], RED);
        message = "'\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
    }
    else {
        char *message = "cp: missing file operand\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
    }
}
