#ifndef THREAD_TOOL_H
#define THREAD_TOOL_H

#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>

// The maximum number of threads.
#define THREAD_MAX 101

void sighandler(int signum);
void scheduler();

// The thread control block structure.
struct tcb
{
    int id;
    int *args;
    // Reveals what resource the thread is waiting for. The values are:
    //  - 0: no resource.
    //  - 1: read lock.
    //  - 2: write lock.
    //  - 3: sleeping.
    int waiting_for;
    int sleeping_time;
    jmp_buf env; // Where the scheduler should jump to.
    int n, i, f_cur, f_prev;
    int p_p, p_s;
};

// The only one thread in the RUNNING state.
extern struct tcb *current_thread;
extern struct tcb *idle_thread;

struct tcb_queue
{
    struct tcb *arr[THREAD_MAX]; // The circular array.
    int head;                    // The index of the head of the queue
    int size;
};

extern struct tcb_queue ready_queue, waiting_queue;

// The rwlock structure.
//
// When a thread acquires a type of lock, it should increment the corresponding count.
struct rwlock
{
    int read_count;
    int write_count;
};

extern struct rwlock rwlock;

// The remaining spots in classes.
extern int q_p, q_s;

// The maximum running time for each thread.
extern int time_slice;

// The long jump buffer for the scheduler.
extern jmp_buf sched_buf;

struct sleeping_thread
{
    struct tcb *thread;
    int wake_time;
};

extern struct sleeping_thread sleeping_set[THREAD_MAX];
extern int sleeping_set_size;

// Directly do functions with weird implementation
// Done thread_create
#define thread_create(func, t_id, t_args) \
    ({                                    \
        do                                \
        {                                 \
            func(t_id, t_args);           \
        } while (0);                      \
    })

#define thread_setup(t_id, t_args)                                                                \
    ({                                                                                            \
        do                                                                                        \
        {                                                                                         \
            struct tcb *thread = (struct tcb *)malloc(sizeof(struct tcb));                        \
            thread->id = t_id;                                                                    \
            thread->args = t_args;                                                                \
            thread->waiting_for = 0;                                                              \
            thread->sleeping_time = 0;                                                            \
            printf("thread %d: set up routine %s\n", t_id, __func__);                             \
            if (setjmp(thread->env) == 0)                                                         \
            {                                                                                     \
                if (t_id == 0)                                                                    \
                {                                                                                 \
                    idle_thread = thread;                                                         \
                    return;                                                                       \
                }                                                                                 \
                else                                                                              \
                {                                                                                 \
                    ready_queue.arr[(ready_queue.head + ready_queue.size) % THREAD_MAX] = thread; \
                    ready_queue.size++;                                                           \
                    return;                                                                       \
                }                                                                                 \
            }                                                                                     \
        } while (0);                                                                              \
    })

#define thread_yield()                                           \
    ({                                                           \
        sigset_t pending;                                        \
        if (setjmp(current_thread->env) == 0)                    \
        {                                                        \
            /* 解鎖並重新鎖定 SIGTSTP 信號 */           \
            sigset_t mask;                                       \
            sigemptyset(&mask);                                  \
                                                                 \
            sigaddset(&mask, SIGTSTP);                           \
            sigprocmask(SIG_UNBLOCK, &mask, NULL);               \
            sigprocmask(SIG_BLOCK, &mask, NULL);                 \
                                                                 \
            /* 解鎖並重新鎖定 SIGALRM 信號 */           \
            sigemptyset(&mask);                                  \
            sigaddset(&mask, SIGALRM);                           \
            sigprocmask(SIG_UNBLOCK, &mask, NULL);               \
            sigprocmask(SIG_BLOCK, &mask, NULL);                 \
                                                                 \
            /* 檢查是否有掛起的信號 */                 \
            sigpending(&pending);                                \
            if (!sigismember(&pending, SIGTSTP) &&               \
                !sigismember(&pending, SIGALRM))                 \
            {                                                    \
                /* 如果沒有信號，返回當前執行緒 */ \
                longjmp(current_thread->env, 1);                 \
            }                                                    \
                                                                 \
            /* 有信號，進入調度器 */                    \
        }                                                        \
    })

#define read_lock()                                   \
    ({                                                \
        while (rwlock.write_count > 0)                \
        {                                             \
            current_thread->waiting_for = 1;          \
            if (setjmp(current_thread->env) == 0)     \
            {                                         \
                longjmp(sched_buf, 200);              \
            }                                         \
            /* Resumed after being scheduled again */ \
        }                                             \
        current_thread->waiting_for = 0;              \
        rwlock.read_count++;                          \
    })

#define write_lock()                                            \
    ({                                                          \
        while (rwlock.read_count > 0 || rwlock.write_count > 0) \
        {                                                       \
            current_thread->waiting_for = 2;                    \
            if (setjmp(current_thread->env) == 0)               \
            {                                                   \
                longjmp(sched_buf, 200);                        \
            }                                                   \
            /* Resumed after being scheduled again */           \
        }                                                       \
        current_thread->waiting_for = 0;                        \
        rwlock.write_count++;                                   \
    })

#define read_unlock()            \
    ({                           \
        do                       \
        {                        \
            rwlock.read_count--; \
        } while (0);             \
    })

#define write_unlock()            \
    ({                            \
        do                        \
        {                         \
            rwlock.write_count--; \
        } while (0);              \
    })

#define thread_sleep(sec)                                                 \
    ({                                                                    \
        do                                                                \
        {                                                                 \
            if (setjmp(current_thread->env) == 0)                         \
            {                                                             \
                current_thread->sleeping_time = sec;                      \
                sleeping_set[current_thread->id].thread = current_thread; \
                sleeping_set[current_thread->id].wake_time = sec;         \
                current_thread->waiting_for = 3;                          \
                sleeping_set_size++;                                      \
                longjmp(sched_buf, 1000);                                 \
            }                                                             \
        } while (0);                                                      \
    })

#define thread_awake(t_id)                                                                                       \
    ({                                                                                                           \
        do                                                                                                       \
        {                                                                                                        \
            if (sleeping_set[t_id].thread != NULL && sleeping_set[t_id].thread->id == t_id)                      \
            {                                                                                                    \
                ready_queue.arr[(ready_queue.head + ready_queue.size) % THREAD_MAX] = sleeping_set[t_id].thread; \
                ready_queue.size++;                                                                              \
                sleeping_set[t_id].thread = NULL;                                                                \
                sleeping_set_size--;                                                                             \
            }                                                                                                    \
                                                                                                                 \
        } while (0);                                                                                             \
    })

#define thread_exit()                                        \
    ({                                                       \
        do                                                   \
        {                                                    \
            printf("thread %d: exit\n", current_thread->id); \
            longjmp(sched_buf, 500);                         \
        } while (0);                                         \
    })

#endif // THREAD_TOOL_H
