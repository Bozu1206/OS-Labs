/*
 * @file fs_util.c
 * @brief Utility functions used to implement the file system
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs_util.h"
#include "open_file_table.h"

int return_sub_inode(char * name, struct lab3_inode * parent, struct lab3_inode * child, bool dir) {

    if (parent == NULL || child == NULL || !parent->is_directory) {
        return -1;
    }

    assert(parent->directory.num_children <= MAX_DATA_BLOCKS_PER_INODE);

    int rez = 0;
    uint32_t child_id = 0;
    struct lab3_inode curr;
    disk_off_t child_offset = 0;

    while (child_id < parent->directory.num_children) {
        child_offset = parent->directory.children_offsets[child_id];
        //Get the data from parent into child
        rez = read_from_disk(child_offset, (void *) &curr, sizeof(struct lab3_inode));

        if (rez < 0) return rez;

        if (strncmp(name, curr.name, MAX_NAME_SIZE - 1) == 0 && ((dir && curr.is_directory) || !dir)) {
            *child = curr;
            return 0; //no error
        }
        ++child_id;
    }
    return -1;
}

struct lab3_inode *find_inode_by_path(const char *path)
{
    if (path == NULL) return NULL;

    //Get the superblock
    struct lab3_superblock * const sblock = get_disk_superblock();
    const disk_off_t  inode_offset =
            (sblock->first_dnode_bmap + sblock->num_dnode_bmap_blocks) * DISK_BLK_SIZE;
    free(sblock);

    //Get the root
    struct lab3_inode parent;
    int error = read_from_disk(inode_offset, &parent, sizeof(struct lab3_inode));

    if (error < 0) return NULL;

    // If we find "/", i.e the root, returns this directly
    if (strcmp(path, "/") == 0) {
        struct lab3_inode * rez = calloc(1, sizeof(struct lab3_inode));
        if (rez == NULL) return NULL;
        *rez = parent;
        return rez;
    }

    //Copy the pathname to use strtok
    char pathname[MAX_NAME_SIZE];
    memset(pathname, 0, MAX_NAME_SIZE);
    strncpy(pathname, path, MAX_NAME_SIZE);

    char * prev = NULL;
    char * curr = strtok(pathname, "/"); // the path /foo/bar becomes foo

    //Iterate through the data structure
    while (curr != NULL) {
        prev = curr;
        curr = strtok(NULL, "/"); //Continue the search by passing NULL
        if (curr != NULL) {
            //Get the next inode
            error = return_sub_inode(prev, &parent, &parent, true);
            if (error < 0) return NULL;
        }
    }

    if (prev == NULL) return NULL;
    else
    {
        struct lab3_inode * rez = calloc(1, sizeof(struct lab3_inode));
        if (rez == NULL) return NULL;

        //Get the last inode into rez
        error = return_sub_inode(prev, &parent, rez, false);
        if (error < 0)
        {
            free(rez);
            return NULL;
        }
        else return rez;
    }
}

void my_mem_cpy(void *dest, void *src, size_t n)
{
    // Typecast src and dest addresses to (char *)
    char *csrc  = (char *) src;
    char *cdest = (char *) dest;

    // Copy contents of src[] to dest[]
    for (int i = 0; i < n; i++)
        cdest[i] = csrc[i];
}

int read_from_disk(disk_off_t disk_offset, void * buffer, size_t size)
{
    if (disk_offset >= DISK_CAPACITY_BYTES || !buffer) return -1;

    const disk_off_t inner_block_offset = disk_offset % DISK_BLK_SIZE;
    const disk_off_t block_ptr = (disk_offset / DISK_BLK_SIZE) * DISK_BLK_SIZE;

    if (size > DISK_BLK_SIZE - inner_block_offset) return -1;

    /**
     * Copy the block into temporal buffer
     */
    char temp[DISK_BLK_SIZE];
    int fetch_block = get_block(block_ptr, temp);
    if (fetch_block < 0) return fetch_block;

    my_mem_cpy(buffer, temp + inner_block_offset, size);
    return 0;
}

/* Implementation of going to the disk, getting block 0, and returning a pointer to
 * a formatted superblock.
 */
struct lab3_superblock *get_disk_superblock(void)
{
    struct lab3_superblock *sblk = (struct lab3_superblock *) malloc(sizeof(struct lab3_superblock));

    /* Read block 0 from the disk */
    int rcode = read_from_disk(0, sblk, sizeof(struct lab3_superblock));
    if (rcode < 0) {
        free(sblk);
        return NULL;
    }

    return sblk;
}

int sanitize_fd_and_size(int fd, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return -1;
    }

    /* Check that there is a file with this descriptor */
    if (open_file_table[fd].inode == NULL) {
        return -1;
    }

    /* size is never negative because it's unsigned */
    if (size >= MAX_DATA_BLOCKS_PER_INODE * DISK_BLK_SIZE) {
        return -1;
    }

    return 0;
}