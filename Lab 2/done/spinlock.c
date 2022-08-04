/**
 * @brief Implementation of spinlock.
 *
 * @author Adrien Ghosn
 */
#include <assert.h>
#include <stdlib.h>
#include "spinlock.h"

void spinlock_init(spinlock_t* lock) {
    *lock = UNLOCKED;
}

void spinlock_lock(spinlock_t* lock) {
    while (__sync_val_compare_and_swap(lock, UNLOCKED, LOCKED) == LOCKED);
}

void spinlock_unlock(spinlock_t* lock) {
    assert(*lock != UNLOCKED);
    while (__sync_bool_compare_and_swap(lock, LOCKED, UNLOCKED) == UNLOCKED);
}
