#include "cp.h"
#include "user-shell.h"
#include "../../ch0/stdlib/string.h"

void cp(char* args_value, int (*args_info)[2], int args_count) {
    if (args_count != 3) {
        char *message = "cp: incorrect number of arguments\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }

    // Extract source file information
    char src_filename[8] = {0};
    char src_extension[3] = {0};
    int src_name_index_first_position = (*(args_info + 1))[0];
    int src_name_length = (*(args_info + 1))[1];
    
    int dot_index = -1;
    for (int i = 0; i < src_name_length; ++i) {
        if (args_value[src_name_index_first_position + i] == '.') {
            dot_index = i;
            break;
        }
    }

    if (dot_index == -1 || dot_index > 8 || src_name_length - 1 - dot_index > 3) {
        char *message = "cp: invalid source file name\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }

    memcpy(src_filename, args_value + src_name_index_first_position, dot_index);
    memcpy(src_extension, args_value + src_name_index_first_position + dot_index + 1, src_name_length - dot_index - 1);

    // Extract destination file information
    char dest_filename[8] = {0};
    char dest_extension[3] = {0};
    int dest_pos_name = (*(args_info + 2))[0];
    int dest_len_name = (*(args_info + 2))[1];
    
    dot_index = -1;
    for (int i = 0; i < dest_len_name; i++) {
        if (args_value[dest_pos_name + i] == '.') {
            dot_index = i;
            break;
        }
    }

    if (dot_index == -1 || dot_index > 8 || dest_len_name - dot_index - 1 > 3) {
        char *message = "cp: invalid destination file name\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }

    memcpy(dest_filename, args_value + dest_pos_name, dot_index);
    memcpy(dest_extension, args_value + dest_pos_name + dot_index + 1, dest_len_name - dot_index - 1);

    // Set up source file request
    uint32_t src_parent_cluster_number = current_directory;
    struct ClusterBuffer cluster_buffer;
    memset(&cluster_buffer, 0, sizeof(struct ClusterBuffer));
    struct FAT32DriverRequest src_request = {
        .buf = &cluster_buffer,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = src_parent_cluster_number,
        .buffer_size = CLUSTER_SIZE
    };
    memcpy(&(src_request.name), src_filename, 8);
    memcpy(&(src_request.ext), src_extension, 3);

    // Read the content of the source file
    int error_code;
    syscalls(0, (uint32_t) &src_request, (uint32_t) &error_code, 0);
    if (error_code != 0) {
        char *message = "cp: cannot read from source file\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }

    // Set up destination file request
    uint32_t dest_parent_cluster_number = current_directory;
    struct FAT32DriverRequest dest_request = {
        .buf = &cluster_buffer,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = dest_parent_cluster_number,
        .buffer_size = CLUSTER_SIZE
    };
    memcpy(&(dest_request.name), dest_filename, 8);
    memcpy(&(dest_request.ext), dest_extension, 3);

    // Write the copied content to the destination file
    syscalls(2, (uint32_t) &dest_request, (uint32_t) &error_code, 0);
    if (error_code != 0) {
        char *message = "cp: cannot copy to destination file\n";
        syscalls(6, (uint32_t) message, strlen(message), RED);
        return;
    }
}
