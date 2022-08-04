/**
 * @file channel.c
 *
 * @brief Implementation of channels.
 *
 * @author Adrien Ghosn
 *
 */
#include <assert.h>
#include "utils.h"
#include "mutex.h"
#include "channel.h"
#include "sys_thread.h"
#include "scheduler_state.h"

void channel_init(channel_t* chan) {
  mutex_init(&chan->mu);
  thread_list_init(&chan->blocked_recv);
  thread_list_init(&chan->blocked_send);
}

void channel_send(channel_t* chan, void* value) {
    mutex_lock(&chan->mu);
    if (!thread_list_is_empty(&chan->blocked_recv)) {
        thread_info_t * head = thread_list_pop(&chan->blocked_recv);
        head->channel_buffer = value;
        tsafe_unblock_thread(head);
        mutex_unlock(&chan->mu);
    } else {
        get_current_thread()->channel_buffer = value;
        thread_list_add(&chan->blocked_send, get_current_thread());
        cond_wait((void *) &chan->mu, MUTEX);
        assert(NULL == get_current_thread()->channel_buffer);
    }
}

void* channel_receive(channel_t* chan) {
    mutex_lock(&chan->mu);
    if (!thread_list_is_empty(&chan->blocked_send)) {
        thread_info_t * head = thread_list_pop(&chan->blocked_send);
        void * val = head->channel_buffer;
        head->channel_buffer = NULL;
        tsafe_unblock_thread(head);
        mutex_unlock(&chan->mu);
        return val;
    } else {
        thread_list_add(&chan->blocked_recv, get_current_thread());
        cond_wait((void *) &chan->mu, MUTEX);
        void * val = get_current_thread()->channel_buffer;
        get_current_thread()->channel_buffer = NULL;
        return val;
    }
}
