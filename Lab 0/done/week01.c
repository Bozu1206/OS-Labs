/**
 * @file week01.c
 * @brief Exercises in C for the first week
 *
 * A few exercises, marked by `TODO`s.
 * 1. Write a string comparison function, with the same semantics as the
 *    standard `strcmp`.
 * 2. Debug the linked-list implementation.
 * 3. Implement binary tree traversals in three orders: in-order, 
 *    preorder, postorder.
 * 4. Count the occurence of letters in a file.
 *
 * @author Atri Bhattacharyya, Adrien Ghosn
 */
#include <stdlib.h>
#include <ctype.h>
#include "week01.h"


int w1_strcmp(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) fprintf(stderr,"Error: null string");

    while(*s1 != '\0') {
        if (*s1 != *s2) break;
        ++s1;
        ++s2;
    }

    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* TODO: Debug the linked list implementation. The API consists of five 
 * functions:
 * - w1_create_node
 * - w1_delete_node
 * - w1_insert_node
 * - w1_remove_node 
 * - w1_size_list
 * Hint: There are two bugs.
 */
w1_node *w1_create_node(int value) {
  w1_node *node = (w1_node *) calloc(1, sizeof(w1_node));

  if(node == NULL) return NULL;
  
  node->element = value;
  node->next = NULL;
  return node;
}

void w1_delete_node(w1_node *node) {
  free(node);
  node = NULL;
}

w1_node *w1_insert_node(w1_node *head, w1_node *node, unsigned pos) {

  if(node == NULL) return NULL;

  /* Inserting into an empty list */
  if(head == NULL){
    /* Only acceptable position is 0 */
    if(pos == 0) return node;
    /* All other positions are illegal */
    else return NULL;
  }

  /* Non-empty list */
  if(pos == 0) {
    node->next = head;
    return node;
  }
  /* Parse through list till position pos */
  w1_node *prev = head;
  for(unsigned i = 1; i < pos; i++){
    prev = prev->next;
    /* This means end of list encountered before pos */
    if(prev == NULL)
      return NULL;
  }
  node->next = prev->next;
  prev->next = node;
  return head;
}

w1_node *w1_remove_node(w1_node *head, w1_node *node) {
  /* Silent failure for empty list */
  if(head == NULL || node == NULL) return NULL;

  /* Removing first node changes head */
  if(node == head) {
     return head->next;
  }
  else {
    w1_node *prev = head;
    while((prev != NULL) && (prev->next != node))
      prev = prev->next;

    /* Silent failure if end of list reached before finding node */
    if(prev == NULL) return NULL;

    prev->next = node->next;
  }
  node->next = NULL;
  return head;
}

unsigned w1_size_list(w1_node *head) {
  unsigned size = 0;

  while(head != NULL) {
    size++;
    head = head->next;
  }
  return size;
}

void w1_print_post_order(Node* node, FILE* fd) {
  if (fd == NULL || node == NULL) return;
    w1_print_post_order(node->left, fd);
    w1_print_post_order(node->right, fd);
    fprintf(fd,"%d ",node->data);
}

void w1_print_pre_order(Node* node, FILE* fd) {
    if (fd == NULL || node == NULL) return;
    fprintf(fd,"%d ",node->data);
    w1_print_pre_order(node->left, fd);
    w1_print_pre_order(node->right, fd);
}
void w1_print_in_order(Node* node, FILE* fd) {
    if (fd == NULL || node == NULL) return;
    w1_print_in_order(node->left, fd);
    fprintf(fd,"%d ",node->data);
    w1_print_in_order(node->right, fd);
}


w1_count_result_t w1_count_letter_freq(const char* file) {
    FILE* fd = fopen(file, "r");
    w1_count_result_t count = calloc(FREQ_LEN, sizeof(w1_freq_t));
    if (count == NULL) fprintf(stderr, "Could not allocate memory.");

    if (fd != NULL)
    {
        int current_char;
        long unsigned read = 0;
        while ((current_char = fgetc(fd)) != EOF)
        {
            ++read;
            if (isalpha(current_char)) {
                count[tolower(current_char) - 'a']++;
            }
            ++current_char;
        }

        fclose(fd);
        for (size_t i = 0; i < FREQ_LEN; ++i)
        {
            if (read != 0) count[i] /= read;
        }
    }
    else
    {
        fprintf(stderr, "Could not open the file.");
    }
  return count;
}