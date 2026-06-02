/*=============================================================
 * FILE:    TCB.h
 * PROJECT: ULTIMA 2.0 - Phase 1 (Scheduler & Semaphore)
 * AUTHOR:  Erik Rodriguez
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Defines the Task Control Block (TCB) structure used by the
 *   Scheduler to track each task's state, identity, and linkage
 *   in the scheduling ring (doubly-linked list).
 *
 *   States:
 *     READY    - Task is eligible to run
 *     RUNNING  - Task is currently executing
 *     BLOCKED  - Task is waiting on a semaphore
 *     TERMINATED - Task has finished and is awaiting cleanup
 *=============================================================*/

#ifndef TCB_H
#define TCB_H

#include <pthread.h>
#include <string>
#include <vector>
#include <cstdint>

// ---- Task State Constants ----
#define READY      0
#define RUNNING    1
#define BLOCKED    2
#define TERMINATED 3   // also used as DEAD in Ultima2.cpp

/*-------------------------------------------------------------
 * struct TCB
 * PURPOSE: Represents one task (thread) in the OS process table.
 *          Stored in a doubly-linked list (scheduling ring).
 *-------------------------------------------------------------*/
struct TCB {
    int         task_id;        // Unique integer ID assigned at creation
    std::string task_name;      // Human-readable name (e.g., "Task 1")
    int         state;          // READY | RUNNING | BLOCKED | TERMINATED
    pthread_t   thread_handle;  // Underlying POSIX thread handle

    TCB *next;  // Next node in the scheduling ring
    TCB *prev;  // Previous node in the scheduling ring
	
	std::vector<int> allowed_targets;			//added for security reasons : security policy used by send/receive
	std::vector<int> allowed_message_types;		//added for security reasons : security policy used by send/receive

    /*----------------------------------------------------------
     * Constructor – initialise all fields to safe defaults.
     *----------------------------------------------------------*/
    TCB(int id, const std::string &name, pthread_t handle)
        : task_id(id),
          task_name(name),
          state(READY),
          thread_handle(handle),
          next(nullptr),
          prev(nullptr)
    {}
};

#endif // TCB_H
