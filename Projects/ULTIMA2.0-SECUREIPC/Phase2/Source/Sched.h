/*=============================================================
 * FILE:    Sched.h
 * PROJECT: ULTIMA 2.0 - Phase 1 (Scheduler & Semaphore)
 * AUTHOR:  Task 2 Implementation
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Declares the Scheduler class.  The Scheduler maintains a
 *   doubly-linked circular list of Task Control Blocks (TCBs),
 *   sometimes called a "scheduling ring".
 *
 *   Public interface required by the assignment:
 *     create_task()     - allocate TCB and insert into ring
 *     kill_task()       - mark a task TERMINATED
 *     yield()           - strict round-robin context switch
 *     garbage_collect() - remove TERMINATED tasks and free memory
 *     dump(int level)   - print process table to stdout
 *=============================================================*/

#ifndef SCHED_H
#define SCHED_H

#include "TCB.h"
#include <pthread.h>
#include <string>
#include <ncurses.h>

class Scheduler {
public:
    /*----------------------------------------------------------
     * Constructor / Destructor
     *----------------------------------------------------------*/
    Scheduler();
    ~Scheduler();

    /*----------------------------------------------------------
     * create_task
     * PURPOSE : Allocates a new TCB, assigns the next available
     *           task ID, and inserts it into the scheduling ring.
     * PARAMS  : task_name  - display name for the task
     *           handle     - pthread_t returned by pthread_create
     * RETURNS : task ID assigned to the new task (>= 1), or -1
     *           on failure.
     *----------------------------------------------------------*/
    int create_task(const std::string &task_name, pthread_t handle);

    /*----------------------------------------------------------
     * kill_task
     * PURPOSE : Marks the TCB with the given ID as TERMINATED.
     *           Does NOT free memory – call garbage_collect().
     * PARAMS  : task_id - ID returned by create_task()
     *----------------------------------------------------------*/
    void kill_task(int task_id);

    /*----------------------------------------------------------
     * yield
     * PURPOSE : Advances the "current" pointer one step around
     *           the ring (strict round-robin), skipping tasks
     *           that are BLOCKED or TERMINATED.  Updates the
     *           state of the outgoing task to READY and the
     *           incoming task to RUNNING.
     *----------------------------------------------------------*/
    void yield();

    /*----------------------------------------------------------
     * garbage_collect
     * PURPOSE : Walks the ring, unlinks every TERMINATED TCB,
     *           and frees its memory.  Safe to call at any time.
     *----------------------------------------------------------*/
    void garbage_collect();

    /*----------------------------------------------------------
     * dump
     * PURPOSE : Prints the contents of the process table to
     *           stdout.  level controls verbosity:
     *             1 = one-line header + table rows
     *             2 = full detail including thread handle
     *----------------------------------------------------------*/
    void dump(WINDOW *win, int level);

    /*----------------------------------------------------------
     * get_state
     * PURPOSE : Returns the current state of the task with the
     *           given ID, or -1 if not found.  Used by the
     *           Semaphore class to check/update task state.
     *----------------------------------------------------------*/
    int get_state(int task_id);

    /*----------------------------------------------------------
     * set_state
     * PURPOSE : Sets the state of the given task.  Used by the
     *           Semaphore class to block / unblock tasks.
     *----------------------------------------------------------*/
    void set_state(int task_id, int new_state);
	
	/*----------------------------------------------------------
	* get_tcb
	* PURPOSE : allows us to get the access to the TCB
	* 			from the ICP since it's gated by the scheduler
	* added for security reasons
	*-----------------------------------------------------------*/
	TCB* get_tcb(int task_id);

private:
    TCB          *ring_head;    // Pointer into the circular list
    TCB          *current;      // TCB currently "running"
    int           next_id;      // Auto-increment ID counter
    pthread_mutex_t sched_lock; // Protects all ring operations

    /*----------------------------------------------------------
     * find_tcb (private helper)
     * PURPOSE : Linear search for a TCB by ID.
     * RETURNS : Pointer to TCB, or nullptr if not found.
     *----------------------------------------------------------*/
    TCB* find_tcb(int task_id);

    /*----------------------------------------------------------
     * state_name (private helper)
     * PURPOSE : Returns a human-readable string for a state int.
     *----------------------------------------------------------*/
    const char* state_name(int state);
};

// Global scheduler instance (declared extern; defined in Sched.cpp)
extern Scheduler scheduler;

#endif // SCHED_H
