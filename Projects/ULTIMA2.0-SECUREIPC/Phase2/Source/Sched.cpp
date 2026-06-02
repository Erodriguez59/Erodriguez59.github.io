/*=============================================================
 * FILE:    Sched.cpp
 * PROJECT: ULTIMA 2.0 - Phase 1 (Scheduler & Semaphore)
 * AUTHOR:  Task 2 Implementation
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Implements the Scheduler class declared in Sched.h.
 *   The process table is a doubly-linked circular list of TCBs.
 *   All public methods are protected by a pthread_mutex so that
 *   the scheduler is thread-safe when called from multiple tasks.
 *=============================================================*/

#include "Sched.h"
#include <iostream>
#include <iomanip>
#include <cstdio>

using namespace std;

// ---- Global instance (shared across all translation units) ----
Scheduler scheduler;

/*=============================================================
 * Constructor
 * Initialises the empty ring and the mutex.
 *=============================================================*/
Scheduler::Scheduler()
    : ring_head(nullptr),
      current(nullptr),
      next_id(1)
{
    pthread_mutex_init(&sched_lock, nullptr);
}

/*=============================================================
 * Destructor
 * Frees all TCBs still in the ring and destroys the mutex.
 *=============================================================*/
Scheduler::~Scheduler()
{
    pthread_mutex_lock(&sched_lock);

    if (ring_head != nullptr) {
        // Break the circular link so we can iterate normally
        TCB *tail = ring_head->prev;
        tail->next = nullptr;

        TCB *cur = ring_head;
        while (cur != nullptr) {
            TCB *tmp = cur->next;
            delete cur;
            cur = tmp;
        }
        ring_head = nullptr;
        current   = nullptr;
    }

    pthread_mutex_unlock(&sched_lock);
    pthread_mutex_destroy(&sched_lock);
}

/*=============================================================
 * create_task
 * Allocates a TCB, links it into the ring, and returns its ID.
 *=============================================================*/
int Scheduler::create_task(const std::string &task_name, pthread_t handle)
{
    pthread_mutex_lock(&sched_lock);

    int id = next_id++;
    TCB *node = new TCB(id, task_name, handle);

    if (ring_head == nullptr) {
        // First node: point to itself (circular)
        ring_head    = node;
        node->next   = node;
        node->prev   = node;
        current      = node;
        node->state  = RUNNING;   // First task starts as RUNNING
    } else {
        // Insert before ring_head (i.e., at the tail of the ring)
        TCB *tail  = ring_head->prev;
        tail->next = node;
        node->prev = tail;
        node->next = ring_head;
        ring_head->prev = node;
        // New tasks enter as READY
        node->state = READY;
    }

    pthread_mutex_unlock(&sched_lock);
    return id;
}

/*=============================================================
 * kill_task
 * Marks the task TERMINATED; actual removal is done by
 * garbage_collect() so we never pull the rug from under a
 * running task.
 *=============================================================*/
void Scheduler::kill_task(int task_id)
{
    pthread_mutex_lock(&sched_lock);

    TCB *node = find_tcb(task_id);
    if (node != nullptr) {
        node->state = TERMINATED;
    }

    pthread_mutex_unlock(&sched_lock);
}

/*=============================================================
 * yield
 * Round-robin: move 'current' to the next READY task.
 * The outgoing task goes back to READY; the incoming goes to
 * RUNNING.  BLOCKED and TERMINATED tasks are skipped.
 *=============================================================*/
void Scheduler::yield()
{
    pthread_mutex_lock(&sched_lock);

    if (current == nullptr || ring_head == nullptr) {
        pthread_mutex_unlock(&sched_lock);
        return;
    }

    // Mark the current task as READY (unless it was just terminated)
    if (current->state == RUNNING) {
        current->state = READY;
    }

    // Walk forward through the ring looking for the next READY task
    TCB *start = current->next;
    TCB *candidate = start;

    do {
        if (candidate->state == READY) {
            break;
        }
        candidate = candidate->next;
    } while (candidate != start);

    // If we found a READY candidate, promote it
    if (candidate->state == READY) {
        current         = candidate;
        current->state  = RUNNING;
    }
    // (If none found – all are BLOCKED or TERMINATED – current stays)

    pthread_mutex_unlock(&sched_lock);
}

/*=============================================================
 * garbage_collect
 * Removes TERMINATED TCBs from the ring and frees their memory.
 * Special care is taken to maintain the circular structure and
 * to update ring_head / current if they point at a dead node.
 *=============================================================*/
void Scheduler::garbage_collect()
{
    pthread_mutex_lock(&sched_lock);

    if (ring_head == nullptr) {
        pthread_mutex_unlock(&sched_lock);
        return;
    }

    // Count nodes first so we know when the ring is fully empty
    int count = 0;
    TCB *p = ring_head;
    do { ++count; p = p->next; } while (p != ring_head);

    TCB *node = ring_head;
    int visited = 0;

    while (visited < count) {
        TCB *next_node = node->next;

        if (node->state == TERMINATED) {
            // Unlink from the ring
            node->prev->next = node->next;
            node->next->prev = node->prev;

            // Fix ring_head / current if they pointed here
            if (node == ring_head) {
                ring_head = (node->next == node) ? nullptr : node->next;
            }
            if (node == current) {
                current = (ring_head == nullptr) ? nullptr : ring_head;
            }

            delete node;
        }

        node = next_node;
        ++visited;
    }

    pthread_mutex_unlock(&sched_lock);
}

/*=============================================================
 * dump
 * Prints the process table in a readable format.
 * level 1 = summary table
 * level 2 = summary table + thread handle (hex)
 *=============================================================*/
void Scheduler::dump(WINDOW *win, int level)
{
    if (win == nullptr) return;

    // LOCK: No other thread can modify the ring OR touch the screen while we draw
    pthread_mutex_lock(&sched_lock);

    //werase(win);
    //wmove(win, 0, 0); // Reset cursor to top-left of the window
    
    wprintw(win, "========== SCHEDULER DUMP (level %d) ==========\n", level);

    if (ring_head == nullptr) {
        wprintw(win, "  [Process table is empty]\n");
        wnoutrefresh(win); // Use wnoutrefresh for multi-window setups
        pthread_mutex_unlock(&sched_lock);
        return;
    }

    // Header logic...
    wprintw(win, "%-12s %-8s %-12s", "Task Name", "ID", "State");
    if (level >= 2) wprintw(win, " %-18s", "Thread Handle");
    wprintw(win, "\n%s\n", std::string(level >= 2 ? 52 : 32, '-').c_str());

    TCB *node = ring_head;
    do {
        wprintw(win, "%-12s %-8d %-12s", 
                node->task_name.c_str(), node->task_id, state_name(node->state));
        if (level >= 2) wprintw(win, " 0x%lx", (unsigned long)node->thread_handle);
        wprintw(win, "\n");
        node = node->next;
    } while (node != ring_head);

    wprintw(win, "==============================================\n");

    /* * CRITICAL: Use wnoutrefresh() then doupdate() 
     * or ensure wrefresh() is inside the lock.
     */
    wrefresh(win); 

    pthread_mutex_unlock(&sched_lock);
}

/*=============================================================
 * get_state  (used by Semaphore)
 *=============================================================*/
int Scheduler::get_state(int task_id)
{
    pthread_mutex_lock(&sched_lock);
    TCB *node = find_tcb(task_id);
    int s = (node != nullptr) ? node->state : -1;
    pthread_mutex_unlock(&sched_lock);
    return s;
}

/*=============================================================
 * set_state  (used by Semaphore to block / unblock tasks)
 *=============================================================*/
void Scheduler::set_state(int task_id, int new_state)
{
    pthread_mutex_lock(&sched_lock);
    TCB *node = find_tcb(task_id);
    if (node != nullptr) {
        node->state = new_state;
    }
    pthread_mutex_unlock(&sched_lock);
}

/*=============================================================
 * find_tcb  (private helper – caller must hold sched_lock)
 *=============================================================*/
TCB* Scheduler::find_tcb(int task_id)
{
    if (ring_head == nullptr) return nullptr;

    TCB *node = ring_head;
    do {
        if (node->task_id == task_id) return node;
        node = node->next;
    } while (node != ring_head);

    return nullptr;
}

/*=============================================================
 * get_tcb  (private helper – gave IPC access to each task TCB)
 * added for security reasons
 *=============================================================*/
 TCB* Scheduler::get_tcb(int task_id) {
	 pthread_mutex_lock(&sched_lock);
	 TCB* node = find_tcb(task_id);
	 pthread_mutex_unlock(&sched_lock);
	 return node;
 }
 
/*=============================================================
 * state_name  (private helper)
 *=============================================================*/
const char* Scheduler::state_name(int state)
{
    switch (state) {
        case READY:      return "Ready";
        case RUNNING:    return "Running";
        case BLOCKED:    return "Blocked";
        case TERMINATED: return "Terminated";
        default:         return "Unknown";
    }
}
