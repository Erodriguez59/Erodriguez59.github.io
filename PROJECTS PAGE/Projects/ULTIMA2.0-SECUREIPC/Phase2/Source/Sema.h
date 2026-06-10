/*=============================================================
 * FILE:    Sema.h
 * PROJECT: ULTIMA 2.0 - Phase 1 (Scheduler & Semaphore)
 * AUTHOR:  Task 2 Implementation
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Declares the Semaphore class.  This is a BINARY semaphore
 *   (value 0 or 1) that blocks tasks rather than busy-waiting.
 *   Blocking is achieved with a pthread_mutex + pthread_cond_t
 *   pair (the same mechanism used in the course labs).
 *
 *   Data members (as required by the assignment spec):
 *     resource_name[64]  - name of the guarded resource
 *     sema_value         - 0 = locked / 1 = available
 *     sema_queue         - queue of task IDs waiting for the lock
 *
 *   Public interface:
 *     down()             - acquire the semaphore (or block)
 *     up()               - release the semaphore (or wake a waiter)
 *     dump(int level)    - print current semaphore state
 *=============================================================*/

#ifndef SEMA_H
#define SEMA_H

#include <pthread.h>
#include <queue>
#include <string>
#include <ncurses.h>

class Semaphore {
public:
    char resource_name[64]; // Name of the resource being managed
    int  sema_value;        // Binary: 1 = free, 0 = taken

    /*----------------------------------------------------------
     * Constructor
     * PARAMS: name  - human-readable resource name
     *                 (default: "screen")
     *----------------------------------------------------------*/
    Semaphore(const char *name = "screen");

    /*----------------------------------------------------------
     * Destructor – cleans up the mutex and condition variable.
     *----------------------------------------------------------*/
    ~Semaphore();

    /*----------------------------------------------------------
     * down  (P operation / "wait")
     * PURPOSE : If the semaphore is free (value == 1), take it
     *           (set value to 0) and return immediately.
     *           If it is taken (value == 0), block the calling
     *           thread on the condition variable until up() wakes
     *           it.  While blocked, the calling task's ID is
     *           recorded in sema_queue and its scheduler state is
     *           set to BLOCKED.
     *----------------------------------------------------------*/
    void down();

    /*----------------------------------------------------------
     * up   (V operation / "signal")
     * PURPOSE : If sema_queue is empty, release the semaphore
     *           (set value to 1).  Otherwise, wake the next
     *           waiting thread and set its scheduler state back
     *           to READY without ever setting value to 1 (hand-
     *           off semantics – the resource stays "taken" but
     *           ownership transfers).
     *----------------------------------------------------------*/
    void up();

    /*----------------------------------------------------------
     * dump
     * PURPOSE : Print resource name, sema_value, and the list
     *           of task IDs in sema_queue.
     * PARAMS  : level - 1 = brief, 2 = verbose
     *----------------------------------------------------------*/
    void dump(WINDOW *win, int level);

private:
    std::queue<pthread_t> sema_queue; // Waiting threads (FIFO order)

    pthread_mutex_t lock;   // Protects sema_value and sema_queue
    pthread_cond_t  ready;  // Signalled by up() to wake a waiter
};

// Global semaphore instance (declared extern; defined in Sema.cpp)
extern Semaphore sema;

#endif // SEMA_H
