/*
 * @file readdir.c
 * @brief Implementation of readdir
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"

void free_list(char **list, uint32_t size) {
    if (list == NULL || size < 0 ) return;

    for (unsigned i = 0; i < size; ++i)
        free(list[i]);
}

int clean_up_error(struct lab3_inode * const inode, char ** const list, uint32_t length, bool clean_list) {
    free(inode);

    if (clean_list) {
        free_list(list, length);
        free(list);
    }

    return -1;
}

int lab3_readdir(const char *path, char ***out, uint32_t *out_size)
{
    if (path == NULL || out == NULL || out_size == NULL) return -1;

    struct lab3_inode * const inode = find_inode_by_path(path);
    if (inode == NULL || !inode->is_directory)
        return clean_up_error(inode, NULL, 0, 0);

    const uint32_t length = inode->directory.num_children;
    char ** const list = calloc(length, sizeof(char *));
    if (list == NULL) return -1;

    //Initialize space inside the list
    for (unsigned i = 0; i < length; ++i) {
        char * ith_name = calloc(MAX_NAME_SIZE, sizeof(char));

        if (ith_name == NULL)
          return clean_up_error(inode, list, length, 1);

        list[i] = ith_name;
    }

    //Read names
    for (unsigned i = 0; i < length; ++i) {
        struct lab3_inode curr;
        int error = read_from_disk(inode->directory.children_offsets[i], &curr, sizeof(struct lab3_inode));

        if (error < 0)
            return clean_up_error(inode, list, length, 1);

        memcpy(list[i], curr.name, MAX_NAME_SIZE);
    }

    *out_size = length;
    *out = list;
    free(inode);
    return 0;
}