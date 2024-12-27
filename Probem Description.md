### Objective
This assignment focuses on implementing a user-level thread library in C. It involves aspects of thread management, including creating, scheduling, and synchronizing threads.

### Specific Functions
Implement thread scheduling, including preemption and context switching.
Manage thread synchronization using read-write locks.
Implement a sleeping and waking mechanism for threads.
Handle thread states, including running, ready, waiting, and sleeping.

### Scope
Completing the macros for thread lifecycle operations in thread_tool.h.
Implementing a signal handler and a scheduler in scheduler.c.
Writing routines that demonstrate thread usage in routine.c.

### Environment Specification
+ OS: x86_64 Linux 6.6
+ C standard: -std=gnu17
  
### File Structure
+ main.c
Contains some functions for initializing the thread library and creating the thread routines. Also, it initializes the data structures and starts the scheduler.
+ thread_tool.h
Provides necessary macros and data structures for thread handling.
+ scheduler.c
Complete a signal handling and a thread scheduling function in this file.
+ routine.c
Contains thread routines.
+ routine.h
Provides function prototypes for routine functions.

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
