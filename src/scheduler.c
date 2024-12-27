#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "routine.h"
#include "thread_tool.h"

int initialized = 0;
int sleeping_set_size = 0;
struct sleeping_thread sleeping_set[THREAD_MAX];

void clear_pending_signals()
{
    struct sigaction ignore_action, old_tstp, old_alrm;
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;

    if (sigaction(SIGTSTP, &ignore_action, &old_tstp) == -1)
    {
        perror("Failed to set TSTP to ignored.\n");
        exit(1);
    }

    if (sigaction(SIGALRM, &ignore_action, &old_alrm) == -1)
    {
        perror("Failed to set ALRM to ignored.\n");
        exit(1);
    }

    if (sigaction(SIGTSTP, &old_tstp, NULL) == -1)
    {
        perror("Failed to restore SIGTSTP handler");
        exit(1);
    }

    if (sigaction(SIGALRM, &old_alrm, NULL) == -1)
    {
        perror("Failed to restore SIGALRM handler");
        exit(1);
    }
}

void update_sleeping_set(int ret)
{
    if (ret != 2)
    {
        return;
    }
    for (int i = 0; i < THREAD_MAX; i++)
    {
        if (sleeping_set[i].thread == NULL)
        {
            continue;
        }
        sleeping_set[i].wake_time -= time_slice;
        if (sleeping_set[i].wake_time <= 0)
        {
            thread_awake(sleeping_set[i].thread->id);
        }
    }
}
void handle_waiting_threads()
{
    int initial_size = waiting_queue.size;
    int index = waiting_queue.head;
    int processed = 0;

    while (processed < initial_size)
    {
        struct tcb *thread = waiting_queue.arr[index];

        if ((thread->waiting_for == 1 && rwlock.write_count == 0) ||
            (thread->waiting_for == 2 && rwlock.read_count == 0 && rwlock.write_count == 0))
        {
            
            ready_queue.arr[(ready_queue.head + ready_queue.size) % THREAD_MAX] = thread;
            ready_queue.size++;
            
            index = (index + 1) % THREAD_MAX;
            waiting_queue.head = index;
            waiting_queue.size--;
        }
        else
        {

            index = (index + 1) % THREAD_MAX;
        }
        processed++;
    }
}
void sighandler(int signum)
{
    if (signum == SIGTSTP)
    {
        printf("caught SIGTSTP\n");
    }
    else
    {
        printf("caught SIGALRM\n");
    }
    longjmp(sched_buf, 2);
}

void scheduler()
{
    int ret = setjmp(sched_buf);

    if (ret == 0)
    {
        for (int i = 0; i < THREAD_MAX; i++)
        {
            sleeping_set[i].thread = NULL;
            sleeping_set[i].wake_time = 0;
        }
        
        if (!initialized)
        {
            initialized = 1;
            thread_create(idle, 0, NULL);
            
        }
    }

    alarm(time_slice);

    
    clear_pending_signals();
    update_sleeping_set(ret);
    handle_waiting_threads();

    
    
    if (ret == 500) 
    {
        free(current_thread->args);
        free(current_thread);
    }
    else if (ret == 1 || ret == 2) 
    {
        if (current_thread->id != idle_thread->id)
        {
            ready_queue.arr[(ready_queue.head + ready_queue.size) % THREAD_MAX] = current_thread;
            ready_queue.size++;
        }
    }
    else if (ret == 200) 
    {
        
        waiting_queue.arr[(waiting_queue.head + waiting_queue.size) % THREAD_MAX] = current_thread;
        waiting_queue.size++;
    }
    else if (ret == 1000) 
    {
        
        
    }

    if (ready_queue.size > 0)
    {

        current_thread = ready_queue.arr[ready_queue.head];
        ready_queue.head = (ready_queue.head + 1) % THREAD_MAX;
        ready_queue.size--;
    }
    else if (sleeping_set_size > 0 || waiting_queue.size > 0)
    {
        current_thread = idle_thread;
    }
    else
    {
        
        free(idle_thread->args); 
        free(idle_thread);
        return;
    }
    
    longjmp(current_thread->env, 1);
}
