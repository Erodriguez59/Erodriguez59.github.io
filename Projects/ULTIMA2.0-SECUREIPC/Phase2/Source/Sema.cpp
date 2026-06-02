/*=============================================================
 * FILE:    Sema.cpp
 * PROJECT: ULTIMA 2.0 - Phase 1 (Scheduler & Semaphore)
 * AUTHOR:  Task 2 Implementation
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Implements the binary Semaphore class declared in Sema.h.
 *
 *   KEY DESIGN DECISION – no busy-wait:
 *     The spec explicitly forbids busy-waiting in down().  We use
 *     a pthread_cond_t so that a blocked thread truly sleeps
 *     (scheduled out by the OS) until up() calls
 *     pthread_cond_signal().
 *
 *   INTEGRATION WITH SCHEDULER:
 *     When a thread blocks, we call scheduler.set_state() to mark
 *     the task BLOCKED in the process table so dump() reflects
 *     reality.  When it is woken, we set it back to READY.
 *     We look up the task ID via the global 'scheduler' object
 *     (which owns the TCB ring).
 *=============================================================*/

#include "Sema.h"
#include "Sched.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdio>

using namespace std;

// ---- Global instance (shared across all translation units) ----
Semaphore sema("screen");

/*=============================================================
 * Constructor
 *=============================================================*/
Semaphore::Semaphore(const char *name)
    : sema_value(1)   // 1 = resource is FREE at construction
{
    // Copy the name safely (null-terminate even on truncation)
    strncpy(resource_name, name, sizeof(resource_name) - 1);
    resource_name[sizeof(resource_name) - 1] = '\0';

    pthread_mutex_init(&lock,  nullptr);
    pthread_cond_init (&ready, nullptr);
}

/*=============================================================
 * Destructor
 *=============================================================*/
Semaphore::~Semaphore()
{
    pthread_cond_destroy (&ready);
    pthread_mutex_destroy(&lock);
}

/*=============================================================
 * down  –  acquire the semaphore (P / wait)
 *
 *   The classic implementation with a condition variable:
 *
 *     lock the mutex
 *     while (sema_value == 0)          // spurious-wakeup safe
 *         cond_wait()                  // atomically releases mutex
 *                                      // and puts thread to sleep
 *     sema_value = 0                   // take the resource
 *     unlock the mutex
 *
 *   We also enqueue the calling thread and mark it BLOCKED in
 *   the scheduler while it is waiting.
 *=============================================================*/
/*=============================================================
* down – acquire the semaphore (P / wait)
*=============================================================*/
void Semaphore::down() {
    pthread_mutex_lock(&lock);

    if (sema_value == 0) {
        sema_queue.push(pthread_self());
        while (sema_value == 0) {
            pthread_cond_wait(&ready, &lock);
        }
    }

    sema_value = 0;
    pthread_mutex_unlock(&lock);
}

/*=============================================================
 * up  –  release the semaphore (V / signal)
 *
 *   If someone is waiting, hand ownership directly to them
 *   (hand-off semantics) without setting sema_value to 1.
 *   Otherwise just mark the semaphore free.
 *=============================================================*/
void Semaphore::up()
{
    pthread_mutex_lock(&lock);

    sema_value = 1;

    if (!sema_queue.empty()) 
    {
        sema_queue.pop();
    }

    pthread_cond_signal(&ready);

    pthread_mutex_unlock(&lock);
	
	/*After releasing resource, call yield(). This will force
	the current task to back of the scheduler ring and allows
	the task we signaled to potentially start immediately.
	*/
	scheduler.yield();
}

/*=============================================================
 * dump
 * Prints the semaphore state to ncurses window.
 *=============================================================*/
void Semaphore::dump(WINDOW *win, int level)
{
	if (!win) return;
	
    pthread_mutex_lock(&lock);

    wprintw(win, "\n---------- SEMAPHORE DUMP (level %d ---\n", level);
    wprintw(win, "Resource: \"%s\"\n", resource_name);
    wprintw(win, "Sema_val: %d (%s)\n", sema_value,
	        (sema_value == 1 ? "FREE" : "TAKEN"));
			
	wprintw(win, "Queue     : ");

    if (sema_queue.empty()) {
        wprintw(win, "[empty]");
    } else {
        // We can't iterate a std::queue non-destructively, so
        // we copy it first.
        queue<pthread_t> tmp = sema_queue;
        bool first = true;
        while (!tmp.empty()) {
            if (!first) wprintw(win, " ---> ");
			//printing thread handle in hex
            wprintw(win, "T-0x%lx", (unsigned long)tmp.front());
            tmp.pop();
            first = false;
        }
    }
    wprintw(win, "\n");

    if (level >= 2) {
        wprintw(win, "Waiting threads: %lu\n", (unsigned long)sema_queue.size());
    }

    wprintw(win, "---------------------------------------------------\n\n");

    pthread_mutex_unlock(&lock);
}
