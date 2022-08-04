/**
 * @brief implementation of mutex functions.
 *
 * @author Adrien Ghosn
 */

#include <assert.h>
#include "utils.h"
#include "mutex.h"
#include "sys_thread.h"

void mutex_init(mutex_t* m) {
  m->owner = DEFAULT_TARGET;
  spinlock_init(&m->lock);
  thread_list_init(&m->blocked);
}

void mutex_lock(mutex_t* m) {
    spinlock_lock(&m->lock);
    if (__sync_bool_compare_and_swap(&m->owner, DEFAULT_TARGET, get_current_thread()->id) == 0) {
        thread_list_add(&m->blocked, get_current_thread());
        cond_wait((void *) &m->lock, SPINLOCK);
        return;
    } else {
        spinlock_unlock(&m->lock);
    }
}

void mutex_unlock(mutex_t* m) {
    assert(m->owner == get_current_thread()->id);
    spinlock_lock(&m->lock);

    if (thread_list_is_empty(&m->blocked)) {
        __sync_bool_compare_and_swap(&m->owner, get_current_thread()->id, DEFAULT_TARGET);
    }
    else {
        thread_info_t * head = thread_list_pop(&m->blocked);
        __sync_bool_compare_and_swap(&m->owner, get_current_thread()->id, head->id);
        tsafe_unblock_thread(head);
    }

    spinlock_unlock(&m->lock);
}


