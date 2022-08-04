/**
 * @file week02.c
 * @brief Exercises in C for week 2
 *
 * TODO's:
 * 1. Complete the implementation of w2_bork()
 * 2. Implement w2_fork()
 * 3. Implement w2_clone()
 *
 * @author Ahmad Hazimeh
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "week02.h"

int check_for_invalid_pid(const pid_t cpid) {
    if (cpid == -1) {
        fprintf(stderr, "fork(): errno %d %s\n", errno, strerror(errno));
        return 1;
    }
    return 0;
}

void join_multiple_threads(pthread_t const * const threads_id, const int number_of_threads) {
    //Join all threads with [int pthread_join(pthread_t thread, void **value_ptr);]
    for (size_t i = 0; i < number_of_threads; ++i) {
        if (pthread_join(threads_id[i], NULL) != 0){
            fprintf(stderr, "An error has occured");
            exit(1);
        }
    }
}

void w2_bork(unsigned int n, void (*verify)(void))
{
    if (n == 0) return;

    pid_t cpid = fork();
    if (check_for_invalid_pid(cpid)) return;

    if (cpid == 0) {
        if (verify != NULL) verify();
        exit(0);
    } else {
        /* This is only executed by the parent process */
        int status;
        waitpid(cpid, &status, 0);
        w2_bork(n - 1, verify);
    }
}

void w2_fork(const bool *root, int index, void (*verify)(void))
{
    if (root == NULL || index < 0) return;

    bool node = root[index];

    if (node)
    {
        pid_t cpid = fork();
        if (check_for_invalid_pid(cpid)) return;

        if (cpid == 0)
        {
            if (verify != NULL) verify();
            w2_fork(root, 2 * index + 1, verify);
            exit(0);
        } else {
            int status;
            waitpid(cpid, &status, 0);
            w2_fork(root, 2 * index + 2, verify);
        }
    }
}

void w2_clone(const struct pnode *root, int index, void * (*verify_thread)(void *), void (*verify_fork)(void))
{
    if (root == NULL || index < 0) return;

    struct pnode node = root[index];
    int node_num_thread = node.num_threads;

    if (node.must_fork)
    {
        if (node_num_thread < 0)
        {
            fprintf(stderr, "Error in the number of threads.");
            exit(1); //1 because of the error
        }

        pthread_t threads_id[node_num_thread];
        // Spawn all the threads with
        // [int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);]
        for (size_t i = 0; i < node_num_thread; ++i) {
            if (verify_thread != NULL)
            {
                if (pthread_create(&threads_id[i], NULL, verify_thread, NULL) != 0) {
                    fprintf(stderr, "An error has occured");
                    exit(1);
                }
            }
        }

        join_multiple_threads(threads_id, node_num_thread);

        pid_t cpid = fork();
        if (check_for_invalid_pid(cpid)) return;

        if (cpid == 0)
        {
            if (verify_fork != NULL)
            {
                verify_fork();
            }

            w2_clone(root, 2 * index + 1, verify_thread, verify_fork);
            exit(0);
        }
        else
        {
            int status;
            waitpid(cpid, &status, 0);
            w2_clone(root, 2 * index + 2, verify_thread, verify_fork);
        }
    }
}
