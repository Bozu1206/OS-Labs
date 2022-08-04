/*
 * @file close.c
 * @brief Implementation of close
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>

#include "fs_api.h"
#include "open_file_table.h"

int lab3_close(int fd)
{
    /*
     * lab3_close closes an open file. Lab3FS stores the list of open files in the open file table, and in order
     * to close a file you just need to remove it from there (i.e. mark the slot in the open file table as invalid).
     * Remember to free the inode structure afterwards, or your implementation will leak memory.
     */

    //Check of bounds
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    struct lab3_open_file * const file = &open_file_table[fd];

    //Can't close twice
    if (file->inode == NULL) return -1;

    free(file->inode);
    file->inode = NULL;
    file->seek_offset = 0;
    return 0;
}