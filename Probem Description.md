### Objective
This assignment focuses on implementing a user-level thread library in C. It involves aspects of thread management, including creating, scheduling, and synchronizing threads.

### Requirements
Your program must match the specified behavior precisely, as the judging environment will strictly compare the output to verify correctness.

### What You Will Do
Implement thread scheduling, including preemption and context switching.
Manage thread synchronization using read-write locks.
Implement a sleeping and waking mechanism for threads.
Handle thread states, including running, ready, waiting, and sleeping.
### Scope
In this assignment, you will be provided with several partially implemented files. Your task is to complete the implementation of a user-level thread library and some thread routines. Specific focus areas include:

Completing the macros for thread lifecycle operations in thread_tool.h.
Implementing a signal handler and a scheduler in scheduler.c.
Writing routines that demonstrate thread usage in routine.c.

### Environment Specification
+ OS: x86_64 Linux 6.6
+ C standard: -std=gnu17
  
### Some Information and Tips
+ In this specification, we use the term longjmp() to refer to both longjmp() and siglongjmp(), setjmp() to refer to both setjmp() and sigsetjmp(). It is part of the assignment to decide which one to use.
+ Modifications to the provided C files are allowed, except for those which are explicitly mentioned in the assignment specification.

### File Structure
Warning: The default version of main.c and routine.h will be used for judging. Your modifications to these files will be discarded.
The source code files is put in the src directory. The following files are provided:
+ main.c
Contains some functions for initializing the thread library and creating the thread routines. Also, it initializes the data structures and starts the scheduler.
+ thread_tool.h
Provides necessary macros and data structures for thread handling. Partially complete; some macros must be completed as specified.
+ scheduler.c
Is blank and requires you to complete a signal handling and a thread scheduling function in this file.
+ routine.c
Contains thread routines. You’ll need to implement parts of this file based on the assignment specifications.
+ routine.h
Provides function prototypes for routine functions. You should not modify this file.
### Pre-defined Functions in main.c
+ This file is complete and will be used as-is during grading. You are not required to understand its implementation, but it is encouraged to read through it to understand the overall program flow.
+ int main(int argc, char *argv[])
The main function that initializes the thread library, creates threads, and starts the scheduler. The parameters will be described in the following sections.
+ void unbuffered_io()
Turns stdin, stdout, and stderr into unbuffered I/O.
+ void init_signal()
This function blocks SIGTSTP and SIGALRM, and then set sighandler() as the signal handler for these two signals.
+ void spawn_thread(int argc, char *argv[])
Creates the user-level threads with the given arguments. You may check why we need the padding variable in the Extra Notes section.
+ void start_threading()
Sets up the alarm, then calls the scheduler.

### User-Level Thread Library Implementation
#### Data Structures
+ Thread Control Block (TCB)
Defined in thread_tool.h as struct tcb
Contains fields for:
int id: Unique identifier for each thread.
int *args: Arguments to be passed to the thread function.
waiting_for: Indicator of which resource the thread is waiting for (e.g., read or write lock).
sleeping_time: Duration for which the thread should sleep.
env: Execution context used for saving and restoring thread state (longjmp usage for context switching).
n, i: Variables to retain values between context switches, allowing the thread to resume computations seamlessly. You can add more fields if needed.
Do not delete existing members of the structure. Routines implemented by TAs will be used for judging, and they may rely on these members.
+ Queues
ready_queue and waiting_queue (both struct tcb_queue): Circular arrays used for managing thread states.
ready_queue: Holds threads that are ready to run.
waiting_queue: Holds threads waiting for a resource to become available.
+ Locks
rwlock (struct rwlock): A read-write lock structure for synchronizing threads.
read_count: Tracks the number of threads holding a read lock.
write_count: Tracks whether a write lock is held (only one thread can hold a write lock).
+ Thread Sleeping Set
A data structure that must be implemented to manage sleeping threads.
Stores threads that are currently sleeping, along with information on how long they should sleep.
When a thread's sleep duration expires, the scheduler shall move it back to the ready_queue.
When a thread_awake() call is made, the awaken thread shall be moved from the sleeping set to the ready_queue despite its remaining sleeping time.
Even though named sleeping_set, it can be implemented as an array or any other data structure. A priority queue is recommended for simulating the real-world implementation.
#### Pre-Defined Global Variables
The following global variables are defined in main.c, and declared as extern in thread_tool.h for use in other files. You can use these variables wherever thread_tool.h is included. Do not "define" global variables in a header files, instead, "declare" them with extern keyword and actually "define" them in a source file.
+ struct tcb *current_thread: Points to the currently running thread.
+ struct tcb *idle_thread: Points to the idle thread, which runs when no other threads are ready to. More details will be discussed later.
+ struct tcb_queue ready_queue: Circular array for storing threads that are ready to run.
+ struct tcb_queue waiting_queue: Circular array for storing threads that are waiting for a resource.
+ struct rwlock rwlock: Read-write lock structure for synchronizing threads.
#### Macros to Implement and Thread Lifecycle
You have to implement the following macros which are essential for managing thread lifecycle and synchronization. Each macro has a specific purpose in the thread lifecycle.
All the behaviors associated with the scheduler will be discussed later.
+ Creation
thread_create(func, t_id, t_args): Creates a new thread with t_id, running the specified function with the given arguments.
Just simply call func with t_id and t_args as arguments.
+ Execution Setup
thread_setup(t_id, t_args): Initializes the TCB, registers the thread in the ready queue (if it is not the idle thread), and prepares it to be scheduled. After that, the control should be returned back to init_threads().
Shall be called at the start of every thread routine to set up the thread.
The following messages should be printed to the standard output:
thread [id]: set up routine [routine name]
The name of the routine can be accessed by the __func__ variable (This is a C99 feature! Refer to this link).
Sets up the initial context for the thread using setjmp(), which will allow the scheduler to switch to this thread later.
If the thread is the idle thread, the idle_thread variable should be set here.
If not, registers the thread in the ready_queue by pushing it into the queue
To give the control back to init_threads(), you should return in this macro, which means return in func() in fact.
+ Yielding Control
thread_yield(): Relinquishes control to the scheduler after each computational iteration or step.
Saves the current thread's context using setjmp(), allowing it to resume later.
Sequentially unblock and block SIGTSTP and SIGALRM signals.
That is, do like this:
Unblock SIGTSTP
Block SIGTSTP
Unblock SIGALRM
Block SIGALRM
setjmp() may be called before unblocking signals, so the thread can safely be preempted by the scheduler.
+ Synchronization (Locks)
All the following macros are associated with the read-write lock rwlock declared in thread_tool.h. Moreover, you can assume that the rwlock is used only to protect exactly one resource in the judging process.
read_lock(): Acquires a read lock of the resource.
If a write lock is held, yields control to the scheduler, which will then push the current thread to the waiting_queue.
You may want to use setjmp() to save the context of the thread.
After switching back to the thread, check if the resource is available again. If so, acquire the read lock; otherwise, yield control to the scheduler again.
In the view of threads, the macro is a blocking call, and they get the lock after returning from the macro.
write_lock(): Acquires a write lock of the resource.
If any read or write lock is held, yields control to the scheduler, which will then push the current thread to the waiting_queue.
You may want to use setjmp() to save the context of the thread.
Similar to read_lock, the macro is a blocking call, and the thread shall get the write lock after returning from the write_lock macro.
read_unlock(): Releases a read lock. It is guaranteed that only one of the owners of the read lock will call the macro in correct implementations based on assignment's requirements.
write_unlock(): Releases a write lock, allowing other threads to acquire the lock. It is also guaranteed that only the owner of the write lock will the macro in correct implementations based on assignment's requirements.
+ Sleeping and Waking
thread_sleep(sec): Puts a thread to sleep for a specified time (sec) and moves it to the sleeping_set.
The duration sec is a positive integer between 1 and 10.
The scheduler will move the thread back to the ready_queue once the sleep duration has elapsed.
We simulate the sleeping time by the number of SIGTSTP and SIGALRM caught times the time slice, not the real-world time.
That is, every time a SIGTSTP or SIGALRM signal is caught, the sleeping time of the thread will be decreased by time_slice. More details will be discussed later.
It simulates that all threads use exactly time_slice seconds in each step.
The real-world time approach will introduce more complexity in the judging process.
The thread shall be put to the sleeping set here but not in the scheduler, as the scheduler will not know the sleep duration.
thread_awake(t_id): Transitions the thread with t_id from the sleeping_set to the ready_queue.
This macro is used by the thread routines to wake up a sleeping thread.
If the thread with t_id is not in the sleeping set, nothing happens.
It is guaranteed that there will not be any invalid t_id passed to this macro in a correct implementation of the thread routines.
+ Termination
thread_exit(): Ends a thread by calling longjmp() to switch to the scheduler, allowing another thread to run.
The following messages should be printed to the standard output:
thread [id]: exit
Note that the thread's resources shall be freed by the scheduler, so just call longjmp() to switch to the scheduler.
### Scheduler Design in scheduler.c
The scheduler is responsible for managing the execution of threads, handling context switching, and ensuring that all threads proceed according to their assigned states (ready, waiting, or sleeping).
Components of the Scheduler
+ Signal Handlers
The prototype of the handler shall be void sighandler(int signum). The function is registered as signal handler in init_signal().
The scheduler uses two signals for thread management:
SIGTSTP: Used for manual context switching, triggered by pressing Ctrl+Z on terminal or the judge.
SIGALRM: Used for automatic context switching, triggered by the alarm() system call to enforce time slices.
Signal handlers switches to the scheduler.
+ Scheduler Function
The main function of the scheduler is to manage thread states, select the next thread to execute, and perform context switching.
Thread State Management: The scheduler moves threads between different states—ready, waiting, and sleeping—based on their conditions and requirements.
Context Switching: The scheduler performs context switching using setjmp() and longjmp(), saving the current thread's environment and jumping to the next thread's saved environment.
+ Thread Queues
The scheduler maintains multiple queues to track the state of threads:
Ready Queue: Holds threads that are ready to run and waiting for their turn.
Waiting Queue: Holds threads that are waiting for a resource, such as a read or write lock.
Sleeping Set: Tracks threads that are sleeping and waiting for their sleep duration to expire.
+ Idle thread
The idle thread is a special thread that runs when no other threads are ready to execute.
The scheduler will schedule the idle thread when there are no threads in the ready queue or waiting queue, but there are still threads in the sleeping set.
For example, consider a scenario where there is only one thread want to sleep for 10 seconds. The time slice is 1 second. The idle thread will be scheduled for 10 times.
To implement the idle thread, a thread with ID 0 shall be created.
The idle thread will be created by the scheduler right after the first time the scheduler called.
The thread routine for the idle thread is idle(), which will be discussed later.
+ Signal Handling
When a SIGALRM or SIGTSTP signal is received, the signal handler is invoked.
When a signal is received, the handler shall print the following message to the standard output:
caught signal [signal name]
The handler proceeds to call longjmp() to switch to the scheduler context.
Scheduler Initialization
On the first call to the scheduler, the following steps are performed:
+ Create the idle thread with ID 0 and the routine idle().
Call setjmp() to save the scheduler's context in schebuf.
+ Scheduler Workflow
Including the first call, the scheduler shall perform the following steps:
Clearing the Pending Signals
The scheduler shall clear any pending SIGTSTP and SIGALRM signals before proceeding.
This may be done by changing the signal handler of SIGTSTP and SIGALRM to SIG_IGN, then restoring the original handler. Refer to Extra Notes if you are interested in the reason.
+ Managing Sleeping Threads
The scheduler checks the sleeping_set to see if any thread's sleep duration has expired.
The sleep duration is decremented by time_slice if we jump to the scheduler from the signal handler.
Your implementation of the sleeping set should be able to handle this.
If a thread's sleep time is over, it is moved from the sleeping_set to the ready_queue, making it eligible for execution.
The order of threads moved from the sleeping set to the ready queue shall be the same as the order of threads' index. For example, if thread 1 and thread 2 are in the sleeping set, and them wake up at the same time, thread 1 shall be put to the ready queue before thread 2.
+ Handling Waiting Threads
The scheduler monitors the availability of resources and moves threads from the waiting_queue to the ready_queue when the resource becomes available.
If the head of the waiting_queue can acquire the resource, it is moved to the ready_queue. Repeat this process until no more threads can be moved.
+ Handling Previously Running Threads
You may use the return value of setjmp() to distinguish where the jump comes from.
If the previous running thread comes from the signal handler, which must be triggered in thread_yield(), then the thread is not waiting for anything and shall be directly pushed to the ready queue.
Note that the idle thread, as a special thread, shall not be pushed to the ready queue.
If the previous running thread comes from a read_lock() or write_lock() call, then the thread is waiting for the resource and shall be pushed to the waiting queue.
If the previous running thread comes from a thread_sleep() call, then nothing to do, the thread is already in the sleeping set.
If the previous running thread comes from a thread_exit() call, then simply free the resources of the thread, including the TCB and the array of arguments, and do not push it back to any queue.
+ Selecting the Next Thread
The scheduler selects the next thread to run from the head of the ready_queue. The thread shall be popped from the queue.
If the ready_queue is empty, it checks if there are any threads in the sleeping_set.
If so, the scheduler schedules the idle thread.
If not, scheduler() shall free the resource of the TCB of the idle thread, then "returns". It actually returns to start_threading(), which will then return back to main().
+ Context Switching
The scheduler performs context switching using the saved environment (env) of each thread.
The next thread's context is restored using longjmp(), which effectively transfers control to that thread.
Routine Functions in routine.c
The routine.c file contains the implementation of various thread routines that are executed by the user-level threads. The prototype of each routine is defined in routine.h. All of them will be in form like:
void routine_name(int id, int *args);
The routines will parse the arguments in their own way. There is no need to check the validity of the arguments, as the arguments will be guaranteed to be valid. args of each routine shall be freed in scheduler(), as mentioned before.
#### Important Note on Judging
+ To ensure that your program runs as expected during the judging process, please include a sleep(1) call before the ending thread_yield() or thread_exit() of each step or iteration in your routines. This will help synchronize the output and make it easier to verify the correctness of your implementation.
+ Initialization
Each routine shall start by calling thread_setup(id, args) at the beginning.
+ Routine Descriptions
Here are the routines that you need to implement in routine.c.
Special Routine: Idle Thread
The idle thread is a special thread that runs when no other threads are ready to execute. The idle thread is created with ID 0 and runs the idle() routine.
No arguments are passed to the idle thread. Thus, you may pass NULL as args. The routine shall do the following:
Print the following message to the standard output:
thread [id]: idle
Call sleep(1).
Call thread_yield().
The thread routine do not call thread_exit() at the end, as the resources of the idle thread will be freed by the scheduler.
+ First Routine: Fibonacci Number
A positive integer $n$ is passed in as the argument, i.e., $𝑛$ can be obtained from args[0]. Your program has to calculate the $𝑛$-th Fibonacci number $𝐹_𝑛$
. Specifically, we define$𝐹_𝑛=$\begin{cases}1,if 𝑛\leq 2\\𝐹_{𝑛−1}+𝐹_{𝑛−2},otherwise\end{cases}$
Compute the answer in exactly $𝑛$ iterations.Overflow is not a concern in this assignment under the given environment.
For the $𝑖$-th iteration, the routine shall do the following:
Calculate the $𝑖$-th Fibonacci number.
Print out the following message to the standard output:
thread [id]: F_[i] = [F_i]
Call sleep(1).
Call thread_yield() or thread_exit(), depending on whether the calculation is finished.
+ Second Routine: Plus and Minus
A positive integer $𝑛$ is passed in as the argument, i.e., $𝑛$ can be obtained from args[0]. Your program has to calculate 
$pm(𝑛)$.
Specifically, we define $pm(𝑛)=\begin{cases}1,if 𝑛=1\\(−1)^{𝑛−1}\cdot 𝑛+pm(𝑛−1),otherwise\end{cases}$
Compute the answer in exactly $𝑛$ iterations.Overflow is not a concern in this assignment under the given environment.
For the $𝑖$-th iteration, the routine shall do the following:
Calculate pm(𝑖).
Print out the following message to the standard output:
thread [id]: pm([i]) = [pm(i)]
Call sleep(1).
Call thread_yield() or thread_exit(), depending on whether the calculation is finished.
+ Third Routine: Enrollment for SP Course
In this routine, you will simulate students attempting to enroll in the 2025 Fall SP course using threads. There are two classes, pj_class and sw_class, both held at the same time. Each student decides which class to enroll in based on:
Their level of desire to enroll in the class, and
The remaining spots (quotas) available in the class.
There are four arguments passed to the routine: $𝑑𝑝$, $𝑑𝑠$, $𝑠$ and $𝑏$
. The arguments are as follows:𝑑𝑝 and 𝑑s represent the student's desire to enroll in pj_class and sw_class, respectively.
𝑠 represents the number of seconds a student will "sleep" at the beginning to simulate students who are oversleeping.
𝑏 represents your best friend's ID, who you will try to awaken after you wake up.
They can be obtained from args[0], args[1], args[2] and args[3], respectively.
The remaining spots in each class are represented by 𝑞𝑝 for pj_class and 𝑞𝑠 for sw_class. The corresponding variables, q_p and q_w, are defined in main.c, and declared as extern in thread_tool.h. They are protected by the read-write lock, rwlock.
Step 1
Print the following message to the standard output:
thread [id]: sleep [s]
Then, Call thread_sleep(s) to simulate oversleeping.
Step 2
Wake up your best friend with ID 𝑏 by calling thread_awake(). Acquire the read lock. Then, record the current values of 𝑞𝑝and 𝑞𝑠
(remaining spots for both classes). Print the following message to the standard output:
thread [id]: acquire read lock
Call sleep(1). Finally, yield.
Step 3
Release the read lock. Then, compute priority values 
𝑝𝑝=𝑑𝑝×𝑞𝑝 and p𝑠=𝑑𝑠×𝑞𝑠to decide which class the student will prioritize for enrollment. Print the following message to the standard output:
thread [id]: release read lock, p_p = [p_p], p_s = [p_s]
Call sleep(1). Finally, yield.
Step 4
Acquire the write lock. Attempt to enroll in the class with the higher 𝑝value. Break ties by enrolling in the class with the higher 
𝑑 value. If the preferred class is already full at this time, enroll in the other class. Then, print the following message to the standard output:
thread [id]: enroll in [class name]
The class name should be pj_class or sw_class. Decrease the remaining quota 𝑞for the enrolled class by one. Call sleep(1). Yield.
It is guaranteed that the 𝑑values will not be equal.
It is guaranteed that the total number of threads executing this routine will not exceed the sum of 𝑞𝑝 and 𝑞𝑠at any time.
Step 5
Release the write lock, then print the following message to the standard output:
thread [id]: release write lock
Call sleep(1). Finally, exit.
#### Compilation and Execution
+ Compilation
To compile the program, run the following command:
make
This command will compile the source files and generate an executable named hw3. Object files files will be stored in the obj directory.
To clean up the compiled files, run:
make clean
This command will remove the hw3 executable and the object file directory.
+ Execution
To run the compiled program, use the following command:
./hw3 time_slice q_s q_p \
    routine_type routine_arg_1 [routine_arg_remaining ...] \
    [routine_type routine_arg_1 [routine_arg_remaining ...]] ...
time_slice specifies the number of seconds a thread can execute before context switching.
routine_type routine_arg_1 [routine_arg_remaining...] specifies the thread routines to be executed. Each routine is defined by its type and arguments.
routine_type is an integer from 1 to 3, corresponding to Fibonacci, Plus and Minus, and Enrollment for SP Course routines, respectively.
The number of routine_args depends on the routine type.
The number of threads will be determined by the number of argument group provided. It is guaranteed that the number of threads will not exceed THREAD_MAX.
