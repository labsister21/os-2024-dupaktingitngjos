#include "mkdir.h"
#include "user-shell.h"
#include "../../ch0/stdlib/string.h"

void mkdir(char* args_value, int (*args_info)[2], int args_count) {
    // If more than 1 argument, throw an error
    if (args_count != 2) {
        char *message = "mkdir: too many arguments\n";
        syscalls(6, (uint32_t)message, strlen(message), RED);
        return;
    }

    // Parse the directory name
    int name_index_first_position = (*(args_info + 1))[0];
    int name_length = (*(args_info + 1))[1];

    // Check if directory name length is more than 8
    if (name_length > 8) {
        char *message = "mkdir: directory name is too long\n";
        syscalls(6, (uint32_t)message, strlen(message), RED);
        return;
    }

    // Set up the request to create the directory
    struct FAT32DriverRequest request = {
        .buf = 0,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = current_directory,
        .buffer_size = 0
    };
    memcpy(request.name, args_value + name_index_first_position, name_length);

    // Make the syscall to create the directory
    int errorCode = 0;
    syscalls(2, (uint32_t) &request, (uint32_t) &errorCode, 0);

    // Handle potential errors from the syscall
    if (errorCode != 0) {
        char *message = "mkdir: cannot create directory '";
        syscalls(6, (uint32_t)message, strlen(message), RED);
        message = args_value + name_index_first_position;
        syscalls(6, (uint32_t)message, name_length, RED);

        switch (errorCode) {
        case 1:
            message = "': Folder exists\n";
            syscalls(6, (uint32_t)message, strlen(message), RED);
            break;
        case 2:
            message = "': Invalid parent cluster\n";
            syscalls(6, (uint32_t)message, strlen(message), RED);
            break;
        case -1:
            message = "': Unknown error occurred\n";
            syscalls(6, (uint32_t)message, strlen(message), RED);
            break;
        default:
            message = "': Invalid error code\n";
            syscalls(6, (uint32_t)message, strlen(message), RED);
            break;
        }
    }
}
