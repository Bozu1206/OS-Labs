/**
 * @file test_malloc.c
 * @brief Unit-test suite for week 5
 *
 * @author Atri Bhattacharyya, Ahmad Hazimeh
 */
#include <check.h>
#include <stdio.h>
#include <stdbool.h>
#include "malloc.h"

void *(*l1_malloc)(size_t) = libc_malloc;
l1_error (*l1_free)(void *) = libc_free;
void (*l1_init)(void) = NULL;
void (*l1_deinit)(void) = NULL;

START_TEST(chunk_malloc_test_1) {
    /* This will test the chunk allocator */
  l1_init = l1_chunk_init;
  l1_deinit = l1_chunk_deinit;
  l1_malloc = l1_chunk_malloc;
  l1_free = l1_chunk_free;

  l1_init();
  void * ptr = l1_malloc(0);
  ck_assert_msg(ptr == NULL, "An allocation of size 0 should fail.");
  l1_deinit();
}
END_TEST


START_TEST(chunk_malloc_test_2) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * ptr = l1_malloc(CHUNK_SIZE);
    ck_assert_msg(ptr == &l1_chunk_arena[1], "An allocation of size 4096 should return the 1st chunk.");
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_3) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    l1_malloc(255 * CHUNK_SIZE);
    void * ptr2 = l1_malloc(CHUNK_SIZE);
    ck_assert_msg(ptr2 == NULL, "An allocation of size 4096 when heap is full should fail.");
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_4) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * ptr = l1_malloc(ALLOC8R_HEAP_SIZE + 1);
    ck_assert_msg(ptr == NULL, "An allocation of size bigger than ALLOC8R_HEAP_SIZE should fail.");
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_5) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * frs = l1_malloc(CHUNK_SIZE * 3);
    void * ptr = l1_malloc(CHUNK_SIZE * 10);
    l1_free(frs);
    ck_assert_msg(ptr == &l1_chunk_arena[5], "First allocate 3 chunks, then 10 chunks, should return the six chunk");
    l1_deinit();
}
END_TEST


START_TEST(chunk_malloc_test_6) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * frs = l1_malloc(CHUNK_SIZE * 3);
    l1_free(frs);
    void * ptr = l1_malloc(CHUNK_SIZE * 10);
    ck_assert_msg(ptr == &l1_chunk_arena[1], "First allocate 3 chunks, then 10 chunks, should return the six chunk");
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_7) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * frs = l1_malloc(CHUNK_SIZE * 3);
    l1_error err = l1_free(frs);
    ck_assert_int_eq((int) SUCCESS, (int) err);
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_8) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    for (int i = 0; i < CHUNK_ARENA_LENGTH >> 1; ++i) {
        ck_assert_int_eq(l1_chunk_meta[i], 0);
    }
    l1_deinit();
}
END_TEST

START_TEST(chunk_malloc_test_9) {
    /* This will test the chunk allocator */
    l1_init = l1_chunk_init;
    l1_deinit = l1_chunk_deinit;
    l1_malloc = l1_chunk_malloc;
    l1_free = l1_chunk_free;

    l1_init();
    void * ptr = l1_malloc(255 * CHUNK_SIZE);
    for (int i = 0; i < CHUNK_ARENA_LENGTH >> 1; ++i) {
        ck_assert_int_eq(l1_chunk_meta[i], 1);
    }
    l1_free(ptr);
    for (int i = 0; i < CHUNK_ARENA_LENGTH >> 1; ++i) {
        ck_assert_int_eq(l1_chunk_meta[i], 0);
    }
    l1_deinit();
}
END_TEST

/* Test malloc with 0 size returns NULL */
START_TEST(list_malloc_test_1) {
  /* This will test the list allocator */
  l1_init = l1_list_init;
  l1_deinit = l1_list_deinit;
  l1_malloc = l1_list_malloc;
  l1_free = l1_list_free;

  l1_init();
  ck_assert_msg(l1_malloc(0) == NULL, "A malloc of size 0 should return NULL.");
  l1_deinit();
}
END_TEST

int main(int argc, char **argv)
{
  Suite* s = suite_create("Threading lab");
  TCase *tc1 = tcase_create("basic tests"); 
  suite_add_tcase(s,tc1);

  tcase_add_test(tc1, chunk_malloc_test_1);
  tcase_add_test(tc1, chunk_malloc_test_2);
  tcase_add_test(tc1, chunk_malloc_test_3);
  tcase_add_test(tc1, chunk_malloc_test_4);
  tcase_add_test(tc1, chunk_malloc_test_5);
  tcase_add_test(tc1, chunk_malloc_test_6);
  tcase_add_test(tc1, chunk_malloc_test_7);
  tcase_add_test(tc1, chunk_malloc_test_8);
  tcase_add_test(tc1, chunk_malloc_test_9);
  tcase_add_test(tc1, list_malloc_test_1);

  /* Add more tests of your own */

  SRunner *sr = srunner_create(s); 
  srunner_run_all(sr, CK_VERBOSE); 

  int number_failed = srunner_ntests_failed(sr); 
  srunner_free(sr); 

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
