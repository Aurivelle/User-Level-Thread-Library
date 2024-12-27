#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "thread_tool.h"

void idle(int id, int *args)
{
    thread_setup(id, args);

    while (1)
    {
        printf("thread %d: idle\n", current_thread->id);

        sleep(1);
        thread_yield();
    }
}

void fibonacci(int id, int *args)
{
    thread_setup(id, args);

    current_thread->n = current_thread->args[0];
    for (current_thread->i = 1;; current_thread->i++)
    {
        if (current_thread->i <= 2)
        {
            current_thread->f_cur = 1;
            current_thread->f_prev = 1;
        }
        else
        {
            int f_next = current_thread->f_cur + current_thread->f_prev;
            current_thread->f_prev = current_thread->f_cur;
            current_thread->f_cur = f_next;
        }
        printf("thread %d: F_%d = %d\n", current_thread->id, current_thread->i,
               current_thread->f_cur);

        sleep(1);

        if (current_thread->i == current_thread->n)
        {
            thread_exit();
        }
        else
        {
            thread_yield();
        }
    }
}

void pm(int id, int *args)
{
    thread_setup(id, args);

    current_thread->n = current_thread->args[0];
    for (current_thread->i = 1;; current_thread->i++)
    {
        if (current_thread->i <= 1)
        {
            current_thread->f_prev = 1;
        }
        else
        {
            int sign = 0;
            if (current_thread->i % 2)
            {
                sign = 1;
            }
            else
            {
                sign = -1;
            }

            int f_next = sign * current_thread->i + current_thread->f_prev;
            current_thread->f_prev = f_next;
        }
        printf("thread %d: pm(%d) = %d\n", current_thread->id, current_thread->i,
               current_thread->f_prev);

        sleep(1);

        if (current_thread->i == current_thread->n)
        {
            thread_exit();
        }
        else
        {
            thread_yield();
        }
    }
}

void enroll(int id, int *args)
{
    thread_setup(id, args);

    printf("thread %d: sleep %d\n", current_thread->id, current_thread->args[2]);
    thread_sleep(current_thread->args[2]);
    thread_awake(current_thread->args[3]);
    read_lock();
    printf("thread %d: acquire read lock\n", current_thread->id);
    sleep(1);
    thread_yield();

    read_unlock();
    current_thread->p_p = q_p * current_thread->args[0];
    current_thread->p_s = q_s * current_thread->args[1];
    printf("thread %d: release read lock, p_p = %d, p_s = %d\n", current_thread->id, current_thread->p_p, current_thread->p_s);
    sleep(1);
    thread_yield();

    write_lock();
    int enrolled = 0;
    if (current_thread->p_p > current_thread->p_s)
    {
        // Prefer pj_class
        if (q_p > 0)
        {
            q_p--;
            printf("thread %d: acquire write lock, enroll in pj_class\n", current_thread->id);
            enrolled = 1;
        }
        else if (q_s > 0)
        {
            q_s--;
            printf("thread %d: acquire write lock, enroll in sw_class\n", current_thread->id);
            enrolled = 1;
        }
    }
    else if (current_thread->p_s > current_thread->p_p)
    {
        // Prefer sw_class
        if (q_s > 0)
        {
            q_s--;
            printf("thread %d: acquire write lock, enroll in sw_class\n", current_thread->id);
            enrolled = 1;
        }
        else if (q_p > 0)
        {
            q_p--;
            printf("thread %d: acquire write lock, enroll in pj_class\n", current_thread->id);
            enrolled = 1;
        }
    }
    else // current_thread->p_p == current_thread->p_s
    {
        // Tie
        if (current_thread->args[0] > current_thread->args[1])
        {
            // Prefer pj_class
            if (q_p > 0)
            {
                q_p--;
                printf("thread %d: acquire write lock, enroll in pj_class\n", current_thread->id);
                enrolled = 1;
            }
            else if (q_s > 0)
            {
                q_s--;
                printf("thread %d: acquire write lock, enroll in sw_class\n", current_thread->id);
                enrolled = 1;
            }
        }
        else
        {
            // Prefer sw_class
            if (q_s > 0)
            {
                q_s--;
                printf("thread %d: acquire write lock, enroll in sw_class\n", current_thread->id);
                enrolled = 1;
            }
            else if (q_p > 0)
            {
                q_p--;
                printf("thread %d: acquire write lock, enroll in pj_class\n", current_thread->id);
                enrolled = 1;
            }
        }
    }

    if (!enrolled)
    {
        printf("thread %d: could not enroll in any class\n", current_thread->id);
    }

    sleep(1);
    thread_yield();
    write_unlock();
    printf("thread %d: release write lock\n", current_thread->id);
    sleep(1);
    thread_exit();
}