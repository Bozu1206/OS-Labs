/**
 * @file sched_policy.c
 * @brief Implementations for custom schedulers
 *
 * @author Atri Bhattacharyya, Mark Sutherland
 */
#include <stdio.h>
#include <stdlib.h>
#include "sched_policy.h"
#include "schedule.h"

/** Round robin just returns the oldest thread in RUNNABLE state */
l1_thread_info* l1_round_robin_policy(l1_thread_info* prev, l1_thread_info* next) {
  if (next != NULL) {
    return next;
  }
  l1_scheduler_info *scheduler = get_scheduler();
  return thread_list_rotate(&scheduler->thread_arrays[RUNNABLE]);
}

/** Schedules the thread with the smallest amount of cycles so far */
l1_thread_info* l1_smallest_cycles_policy(l1_thread_info* prev, l1_thread_info* next) {
  if (next != NULL) return next;

  l1_scheduler_info *scheduler = get_scheduler();
  l1_thread_list* list_of_runnable = &scheduler->thread_arrays[RUNNABLE];
  if (list_of_runnable->head == NULL) return NULL;

  //Find smallest cycle
  l1_thread_info * current = list_of_runnable->head;
  l1_thread_info * smallest = current;
  while (current->next != NULL) {
      current = current->next;
      if (l1_time_is_smaller(current->total_time, smallest->total_time)) {
          smallest = current;
      }
  }
  return smallest;
}


void l1_mlfq_boost_of_unrunned_thread(l1_thread_list * list_of_thread) {
    if (list_of_thread->head == NULL) return;

    l1_thread_info * curr = list_of_thread->head;
    const l1_thread_info * const rest = list_of_thread->tail;

    // Boost thread that did not run
    while(curr != rest) {
        curr = curr->next;
        if (!curr->got_scheduled) {
            l1_time_init(&curr->total_time); //reset the total_time as indicated
            l1_priority_increase(&curr->priority_level);
        }
    }
}

l1_thread_info * l1_next_thr_to_run(l1_thread_list* list_of_thread) {
    if (list_of_thread->head == NULL) return NULL;

    const l1_thread_info * const rest = list_of_thread->tail;
    l1_thread_info * current = list_of_thread->head;
    l1_thread_info * high_prio = current;

    // Find the thread with The Highest Priority
    while(current != rest) {
        current = current->next;
        if (current->priority_level > high_prio->priority_level) {
            high_prio = current;
        }
    }
    return high_prio;
}

void l1_mlfq_demote_thread(l1_thread_info* prev) {
    l1_time slice_time, slice_size_level;
    l1_time_init(&slice_time);

    // Slice time usage:
    l1_time_diff(&slice_time, prev->slice_end, prev->slice_start);

    l1_time_init(&slice_size_level);
    slice_size_level = l1_priority_slice_size(prev->priority_level);

    //Check for usage :
    const int time_usage_smaller = l1_time_is_smaller(slice_size_level, slice_time);
    const int time_usage_equal   = l1_time_are_equal(slice_size_level, slice_time);
    const int threshold  = l1_time_is_smaller(TIME_PRIORITY_THRESHOLD, prev->total_time);

    if (time_usage_smaller || time_usage_equal || threshold) {
        prev->got_scheduled = 0; //De-schedule the thread
        l1_time_init(&prev->total_time);
        l1_priority_decrease(&prev->priority_level);
    }
}


/** Schedules threads according to mlfq policy */
l1_thread_info* l1_mlfq_policy(l1_thread_info* prev, l1_thread_info* next) {
    l1_scheduler_info * sched = get_scheduler();
    l1_thread_list* list_of_runnable = &sched->thread_arrays[RUNNABLE];

    /** RULE 1 : if priority(A) > priority(B) then return A */
    // Done in next_thr_to_run

    /** RULE 2 : if priority(A) == priority(B) then A, B run in round robin */
    // Done in next_thr_to_run

    /** RULE 3  : All threads should be initialized with top priority. */
    // Done in thread.c

    /** RULE 5 : If a thread has not been run for some time, increase its priority by one level.
    //          In our case, a boost should occur once every SCHED_PERIOD calls to schedule. */
    if (!sched->sched_ticks) l1_mlfq_boost_of_unrunned_thread(list_of_runnable);

    /** RULE 4 : If a thread uses up its whole time slice, demote it to lower priority
            (time slices are longer at lower priority). **/
    // Done in mlfq_demote_thread

    /** RULE 6 : If a thread's total time at its current priority level (not just the last time slice)
    //          has exceeded a threshold time, demote it to a lower priority. */
    // Done in mlfq_demote_thread

    l1_mlfq_demote_thread(prev);
    return l1_next_thr_to_run(list_of_runnable);
}

