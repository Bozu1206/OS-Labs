/*
 * @file read.c
 * @brief Implementation of read
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <string.h>
#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int32_t lab3_read(int fd, void *buffer, uint32_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || buffer == NULL) return -1;

    if (size < 0 || size >= DISK_CAPACITY_BYTES) return -1;

    struct lab3_open_file * const open_file = &open_file_table[fd];

    if (open_file == NULL || open_file->inode->is_directory) return -1;

    if (open_file->seek_offset >= open_file->inode->file.size) return 0;

    if (sanitize_fd_and_size(fd, size)) return -1;

    uint32_t const remaining  = open_file->inode->file.size - open_file->seek_offset;
    uint32_t to_read          = (size <= remaining) ? size : remaining;
    uint32_t block            = open_file->seek_offset / DISK_BLK_SIZE;
    uint32_t offset           = open_file->seek_offset % DISK_BLK_SIZE;
    uint32_t remaining_in_blk = DISK_BLK_SIZE - offset;
    uint32_t current_offset   = open_file->inode->file.data_block_offsets[block];

    if (remaining_in_blk < to_read) {
        //Cross block, start by finishing the current block
        int error = read_from_disk(current_offset + offset, buffer, remaining_in_blk);
        if (error < 0) return -1;

        if (open_file->seek_offset >= open_file->inode->file.size) return remaining_in_blk;

        //Find the end of the buffer
        buffer = ((char *) buffer) + remaining_in_blk;
        uint32_t rem_to_read_after_1st_block = to_read - remaining_in_blk;
        current_offset = open_file->inode->file.data_block_offsets[++block];

        while (rem_to_read_after_1st_block >= DISK_BLK_SIZE && error == 0) {
            //Read the next block
            error = read_from_disk(current_offset, buffer, DISK_BLK_SIZE);
            if (error < 0) return -1;

            if (open_file->seek_offset >= open_file->inode->file.size)
                return rem_to_read_after_1st_block;

            //update pointer and the number of bytes to read
            buffer = ((char *) buffer) + DISK_BLK_SIZE;
            rem_to_read_after_1st_block -= DISK_BLK_SIZE;
            current_offset = open_file->inode->file.data_block_offsets[++block];
        }
        //If we have to read some other bytes but not an entire blk
        if (rem_to_read_after_1st_block > 0) {
            error = read_from_disk(current_offset, buffer, rem_to_read_after_1st_block);
            if (error < 0) return -1;
        }
    } else {
        //Just reed at most one block
        const int error = read_from_disk(current_offset + offset, buffer, to_read);
        if (error < 0) return -1;
    }
    open_file->seek_offset += to_read;
    return to_read;
}