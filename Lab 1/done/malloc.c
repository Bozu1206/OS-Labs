/**
 * @file malloc.c
 * @brief Implementations of custom allocators
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "malloc.h"
#include "error.h"

//For ceil function
#include <math.h>

/*********************** Standard GLIBC malloc ***********/
void *libc_malloc(size_t size) {
    return malloc(size);
}

l1_error libc_free(void *ptr) {
    free(ptr);
    return SUCCESS;
}
/**********************************************************/
/*********************** Chunk malloc *********************/
char (*l1_chunk_arena)[CHUNK_SIZE];
l1_chunk_desc_t *l1_chunk_meta;
max_align_t l1_region_magic;

void l1_chunk_init(void)
{
    /* Allocate chunk arena and metadata */
    l1_chunk_arena = malloc(CHUNK_ARENA_LENGTH * CHUNK_SIZE);
    l1_chunk_meta  = calloc(CHUNK_ARENA_LENGTH, sizeof(l1_chunk_desc_t));
    if ((l1_chunk_arena == NULL) || (l1_chunk_meta == NULL)) {
        printf("Unable to allocate %d bytes for the chunk allocator\n", ALLOC8R_HEAP_SIZE);
        exit(1);
    }

    /* Generate random chunk magic */
    srand(time(NULL));
    for(unsigned i = 0; i < sizeof(max_align_t); ++i)
        *(((char *)&l1_region_magic) + i) = rand();
}

void l1_chunk_deinit(void)
{
    free(l1_chunk_meta);
    l1_chunk_meta = NULL;
    free(l1_chunk_arena);
    l1_chunk_arena = NULL;
}

int l1_are_multiple_chunk_free(const size_t start, const size_t end, size_t *index_not_free_start) {
    if (index_not_free_start == NULL) return 0;

    int free = 1;
    size_t i = start;
    while (i < start + end) {
        if (IS_CHUNK_TAKEN(i)) {
            free = 0;
            *index_not_free_start = i;
        }
        ++i;
    }
    return free;
}

void l1_init_header(const size_t start, const size_t total_size) {
    l1_region_hdr_t header;
    header.size   = total_size;
    header.magic0 = l1_region_magic;
    header.magic1 = l1_region_magic;

    //As indicated on the forum
    memcpy(l1_chunk_arena[start], &header, sizeof(l1_region_hdr_t));
}

void l1_alloc_chunk(const size_t start, const size_t bound) {
    for (size_t i = start; i < bound ; ++i) {
        SET_CHUNK_TAKEN(i);
    }
}

#define NUMBER_OF_CHUNK(SIZE) (1 + (ceil((SIZE) / (CHUNK_SIZE))))
void *l1_chunk_malloc(size_t size)
{
    if (size == 0) return NULL;
    if (size > ALLOC8R_HEAP_SIZE) {
        l1_errno = ERRNOMEM;
        return NULL;
    }

    int is_allocable = 0;
    size_t index_allocation = 0;
    const size_t chunk_to_allocate = NUMBER_OF_CHUNK(size);
    const size_t chunk_bound = CHUNK_ARENA_LENGTH - chunk_to_allocate + 1;

    for (size_t i = 0; (!is_allocable && i < chunk_bound); ++i) {
        if (IS_CHUNK_FREE(i)) {
            size_t index_not_free_start = 0;
            if (l1_are_multiple_chunk_free(i, chunk_to_allocate, &index_not_free_start)) {
                is_allocable = 1;
                index_allocation = i;
            }
            i = index_not_free_start + 1;
        }
    }

    if (is_allocable) {
        l1_alloc_chunk(index_allocation, index_allocation + chunk_to_allocate);
        l1_init_header(index_allocation, size);
        return &(l1_chunk_arena[index_allocation + 1]);
    }

    l1_errno = ERRNOMEM;
    return NULL;
}

int l1_align(max_align_t *x, max_align_t *y) {
    if (x == NULL || y == NULL) return 0;
    //Use memcmp as suggered on the forum
    return (memcmp(x, y, sizeof(max_align_t)) == 0 ? 1 : 0);
}

void l1_free_chk(const size_t bound, const size_t offset) {
    for (size_t i = 0; i < bound; ++i) {
        SET_CHUNK_FREE(i + offset);
    }
}

l1_error l1_chunk_free(void *ptr)
{
    if (ptr == NULL) return SUCCESS;
    if (ptr < (void *) l1_chunk_arena) return ERRINVAL;

    //Get chunks
    char (*chunks)[CHUNK_SIZE] = ptr;
    //Get header
    char (*header_chunk)[CHUNK_SIZE] = &chunks[-1];

    l1_region_hdr_t header;
    memcpy(&header, header_chunk, sizeof(l1_region_hdr_t));

    if (!l1_align(&header.magic0, &l1_region_magic)) return ERRINVAL;
    if (!l1_align(&header.magic1, &l1_region_magic)) return ERRINVAL;

    //Free all the chunks even the header
    size_t offset      = (header_chunk - l1_chunk_arena);
    size_t chunk_count = NUMBER_OF_CHUNK(header.size);
    l1_free_chk(chunk_count, offset);
    return SUCCESS;
}

/**********************************************************/
/****************** Free list based malloc ****************/
l1_list_meta *l1_list_free_head = NULL;
void *l1_list_heap = NULL;
max_align_t l1_list_magic;

#define OFFSET offsetof(l1_list_meta, next)
void l1_init_new_meta(l1_list_meta * meta, size_t current_capacity, size_t size, l1_list_meta * next) {
    // Notice that the size/capacity specified in the region's header does not
    // include the space occupied by the metadata storing the next pointer.
    meta->capacity = current_capacity - OFFSET - size;
    meta->magic1   = l1_list_magic;
    meta->magic0   = l1_list_magic;
    meta->next     = next;
}

void l1_list_init() {
    l1_list_heap = malloc(ALLOC8R_HEAP_SIZE);
    if(l1_list_heap == NULL) {
        printf("Unable to allocate %d bytes for the list\n", ALLOC8R_HEAP_SIZE);
        exit(1);
    }

    /* Generate random list magic */
    srand(time(NULL));
    for(unsigned i = 0; i < sizeof(max_align_t); i++)
        *(((char *)&l1_list_magic) + i) = rand();

    l1_list_meta metadata;
    l1_init_new_meta(&metadata, ALLOC8R_HEAP_SIZE, 0, NULL);

    //Copy the header (meta) in the heap
    memcpy(l1_list_heap, &metadata, sizeof(l1_list_meta));
    l1_list_free_head = l1_list_heap;
}

void l1_list_deinit() {
    free(l1_list_heap);
    l1_list_heap = NULL;
}

void l1_make_list( l1_list_meta *curr, l1_list_meta *prev, l1_list_meta *up)
{
    if (curr == NULL && prev == NULL && up == NULL) return;
    if (curr != l1_list_free_head) prev->next = up;
    else l1_list_free_head = up;
}

#define SZ_ALI sizeof(max_align_t)
void *l1_list_malloc(size_t req_size)
{
    if (req_size == 0) return NULL;
    if (req_size > ALLOC8R_HEAP_SIZE - OFFSET) {
        l1_errno = ERRNOMEM;
        return NULL;
    }

    const size_t size = SZ_ALI * (req_size / SZ_ALI + (req_size % SZ_ALI == 0 ? 0 : 1));

    l1_list_meta * curr = l1_list_free_head;
    if (curr == NULL) return NULL;

    l1_list_meta * prev = NULL;
    while (curr != NULL && curr->capacity < size) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return NULL;
    if (size <= curr->capacity - sizeof(l1_list_meta)) {
        // New meta
        l1_list_meta up_meta;
        l1_init_new_meta(&up_meta, curr->capacity, size, curr->next);
        l1_list_meta * up_meta_pos = (l1_list_meta *) (((char *) curr)  + OFFSET + size);
        memcpy(up_meta_pos, &up_meta, sizeof(l1_list_meta));

        curr->capacity = size;
        l1_make_list(curr, prev, up_meta_pos);
    } else l1_make_list(curr, prev, curr->next);

    return (void *) ((char *) curr  + OFFSET);
}

l1_error l1_list_free(void *ptr) {
    if (ptr == NULL) return SUCCESS;
    if (ptr < (void *) ALLOC8R_HEAP_SIZE) return ERRINVAL;

    l1_list_meta * meta = (l1_list_meta *) (((char *) ptr) - OFFSET);

    if (!l1_align(&meta->magic0, &l1_list_magic)) return ERRINVAL;
    if (!l1_align(&meta->magic1, &l1_list_magic)) return ERRINVAL;

    l1_list_free_head = meta;
    return SUCCESS;
}