/*
 * @file open.c
 * @brief Implementation of open
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>

#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"
#include <string.h>

static inline int clean_up_error(struct lab3_inode * const inode) {
    free(inode);
    return -1;
}

int lab3_open(const char *path)
{
    if (path == NULL) return -1;

    //Get a copy of the inode of the file to open
    struct lab3_inode * const inode = find_inode_by_path(path);

    //Check if the file exist, if it's not a directory
    if (inode == NULL || inode->is_directory || path[0] != '/')
        return clean_up_error(inode);


    int file_descriptor = -1;
    struct lab3_open_file * entry_table = NULL;
    struct lab3_open_file * free_entry  = NULL;

    //Iterate over the open file table and find a free slot
    for (unsigned i = 0; i < MAX_OPEN_FILES; i++) {
        entry_table = &open_file_table[i];
        if (entry_table->inode != NULL) {
            //Check that the file is not already open
            if (entry_table->inode->id == inode->id)
                return clean_up_error(inode);
        }
        else
        {
            //Free slot found at index i
            file_descriptor = i;
            free_entry = entry_table;
        }
    }

    if (free_entry == NULL)
        return clean_up_error(inode);


    free_entry->inode = inode;
    //Start to read the file at the beginning
    free_entry->seek_offset = 0;
    return file_descriptor;
}
