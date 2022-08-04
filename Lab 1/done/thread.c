/**
 * @file week03.c
 * @brief Outlines in C for student week 3
 *
 * @author Mark Sutherland
 */
#include <stdlib.h>

#include "schedule.h"
#include "stack.h"
#include "thread.h"
#include "thread_info.h"

void l1_start(void) {
  l1_thread_info* cur = get_scheduler()->current;
  /* enter execution */
  void* ret = cur->thread_func(cur->thread_func_args); 
  cur->retval = ret; 
  cur->state = ZOMBIE;
  /* Let the scheduler do the cleanup */
  yield(-1); 
}

l1_error l1_init_stack(l1_thread_info * thread) {
    l1_stack * new_stack = l1_stack_new();
    if (new_stack == NULL) return ERRNOMEM;

    l1_stack_push(new_stack, (uint64_t) 0);
    l1_stack_push(new_stack, (uint64_t) l1_start);

    //Need to push 6 registers on stack
    for (size_t i = 0; i < 6; ++i) {
        l1_stack_push(new_stack, (uint64_t) 0);
    }

    thread->thread_stack = new_stack;
    return SUCCESS;
}

l1_error l1_thread_create(l1_tid *thread, void *(*start_routine)(void *), void *arg) {
  l1_tid new_tid = get_uniq_tid();
  l1_thread_info * new_thread = calloc(1, sizeof(l1_thread_info));
  if (new_thread == NULL) return ERRNOMEM;

  new_thread->id = new_tid;
  new_thread->state = RUNNABLE;

  /** Stack */
  l1_init_stack(new_thread);

  new_thread->thread_func  = start_routine;
  new_thread->thread_func_args = arg;
  new_thread->joined_target = -1;
  new_thread->yield_target  = -1;
  new_thread->errno  = SUCCESS;
  new_thread->join_recv = NULL;
  new_thread->prev = NULL;
  new_thread->next = NULL;

  //Week 4
  new_thread->got_scheduled  = 0;
  new_thread->priority_level = TOP_PRIORITY;
  l1_time_init(&new_thread->total_time);
  l1_time_init(&new_thread->slice_start);
  l1_time_init(&new_thread->slice_end);

  add_to_scheduler(new_thread, new_thread->state);

  /* Give the user the right thread id */
  *thread = new_tid;

  return SUCCESS;
}

l1_error l1_thread_join(l1_tid target, void **retval) {
  /* Setup necessary metadata and block yourself */
  l1_scheduler_info  *scheduler_info = get_scheduler();
  l1_thread_info     *current_thread = scheduler_info->current;

  current_thread->state = BLOCKED;
  current_thread->joined_target = target;
  current_thread->join_recv = retval;
  current_thread->errno = SUCCESS;

  yield(-1);

  l1_error yield_result = current_thread->errno;

  current_thread->joined_target = -1;
  current_thread->join_recv = NULL;
  current_thread->errno = SUCCESS;

  return yield_result;
}
