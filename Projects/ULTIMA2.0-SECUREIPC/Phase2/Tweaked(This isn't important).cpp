/*=============================================================
 * FILE:    Ultima2.cpp
 * PROJECT: ULTIMA 2.0 - Phase 2 (Message Passing)
 * AUTHOR:  Erik Rodriguez, Alvaro Espinoza
 * DATE:    Spring 2026
 * 
 * COMPILE: g++ Ultima2.cpp IPC.cpp Sched.cpp Sema.cpp -o ultima -lpthread -lncursesw
 *
 * DESCRIPTION:
 *   Main entry point for the ULTIMA 2.0 simulated OS.
 *   Boots the OS, creates three tasks (each writing to its own
 *   ncurses window), and runs an input loop so the user can
 *   kill tasks individually or quit cleanly.
 *
 *   BUG FIXES from the original skeleton:
 *     1. sprint() -> sprintf()
 *     2. nodelay(stdsct, TRUE) -> nodelay(stdscr, TRUE)
 *     3. case '1': td.kill_signal -> td1.kill_signal
 *     4. Uncommented the Scheduler and Semaphore calls
 *     5. Added Task 2 and Task 3 creation (were empty comments)
 *     6. create_task() now receives the pthread_t handle so the
 *        scheduler can build a complete TCB.
 *     7. Replaced hardcoded kill_signal references with
 *        TaskSignalTable + launch_task() paging system.
 *=============================================================*/

#include <iostream>
#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <ncurses.h>
#include <sstream>
#include <unordered_map>

#include "Sched.h"
#include "Sema.h"
#include "IPC.h"

using namespace std;

const int NORMAL_MSG = 0;
constexpr size_t MAX_MSG_LEN = 255;
// Global IPC
ipc messenger(3);

/*-------------------------------------------------------------
 * thread_data
 * Temporary per-task data packet passed to the thread function.
 *-------------------------------------------------------------*/
struct thread_data {
    int     thread_id;
    int     thread_state;
    WINDOW *thread_win;
    bool    kill_signal;
    int     sleep_time;
};

/*=============================================================
 * TaskSignalTable
 * Paging registry: maps task ID -> thread_data*.
 * Single responsibility: dispatch kill signals.
 *
 * register_page() : O(1) avg
 * signal_kill()   : O(1) avg, O(n) worst
 *=============================================================*/
class TaskSignalTable {
public:
    void register_page(int id, thread_data* td) {
        table_[id] = td;
    }

    void signal_kill(int id) {
        auto it = table_.find(id);
        if (it != table_.end())
            it->second->kill_signal = true;
    }

private:
    unordered_map<int, thread_data*> table_;
};

// Global signal table
TaskSignalTable signal_table;

// Forward declaration
void* perform_simple_output(void* arguments);

/*=============================================================
 * launch_task
 * Single registration point: registers the td page, then
 * creates the thread. Adding a new task only ever needs one
 * call here — no other code needs to change.
 *=============================================================*/
void launch_task(pthread_t* handle, thread_data* td)
{
    signal_table.register_page(td->thread_id, td);
    pthread_create(handle, NULL, perform_simple_output, td);
}

/*=============================================================
 * create_window
 *=============================================================*/
WINDOW *create_window(int height, int width, int starty, int startx)
{
    WINDOW *win;

    sema.down();

    win = newwin(height, width, starty, startx);
    scrollok(win, TRUE);
    box(win, 0, 0);
    wrefresh(win);

    sema.up();

    return win;
}

/*=============================================================
 * write_window
 *=============================================================*/
void write_window(WINDOW *win, const char *text)
{
    sema.down();

    int y, x;
    getyx(win, y, x);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (x == 0) x = 1;

    if (x >= max_x - 2) {
        y++;
        x = 1;
    }

    wmove(win, y, x);
    wprintw(win, "%s", text);
    box(win, 0, 0);
    wrefresh(win);

    sema.up();
}

/*=============================================================
 * dump_system
 *=============================================================*/
void dump_system(WINDOW *win)
{
    sema.down();

    werase(win);
    box(win, 0, 0);

    int row = 1;

    mvwprintw(win, row++, 2, "===Scheduler===");
    scheduler.dump(win, 1);

    row += 6;

    mvwprintw(win, row++, 2, "===IPC Mailboxes===");

    for (int i = 0; i < 3; i++) {
        int count = messenger.Message_Count(i);
        mvwprintw(win, row++, 2, "Task %d: %d messages", i+1, count);
    }

    row++;
    messenger.ipc_Message_Dump(win, row);

    wrefresh(win);
    sema.up();
}

/*=============================================================
 * perform_simple_output
 *=============================================================*/
void* perform_simple_output(void *arguments)
{
    thread_data *td = (thread_data*)arguments;

    int  id          = td->thread_id;
    int  cpu_quantum = 0;
    char buff[256];

    while (!td->kill_signal) {
        scheduler.yield();

        sprintf(buff, "Hello from T%d (#%d)", id, cpu_quantum);

        int sent   = 0;
        int denied = 0;

        for (int i = 1; i <= 3; i++) {
            int dest = ((id + i - 1) % 3) + 1;

            if (dest == id) continue;

            int result = messenger.Message_Send(id-1, dest-1, buff, NORMAL_MSG);

            if (result == 1) {
                sprintf(buff, "Sent -> Task %d\n", dest);
                write_window(td->thread_win, buff);
                sent = 1;
                break;
            }
            else if (result == -2) {
                denied = 1;
            }
        }

        if (!sent) {
            if (denied)
                write_window(td->thread_win, "SECURITY: send denied\n");
            else
                write_window(td->thread_win, "No alive tasks to send to\n");
        }

        char msg[256];
        int  type;

        int r = messenger.Message_Receive(id-1, msg, sizeof(msg), &type);

        if (r == 1) {
            sprintf(buff, "Received: %s\n", msg);
            write_window(td->thread_win, buff);
        }

        sleep(td->sleep_time);
        cpu_quantum++;
    }

    td->thread_state = TERMINATED;

    sprintf(buff, "Task %d TERMINATED\n", id);
    write_window(td->thread_win, buff);

    scheduler.kill_task(id);
    messenger.Message_DeleteAll(id - 1);

    return NULL;
}

/*=============================================================
 * main
 *=============================================================*/
int main()
{
    pthread_t   thread1, thread2, thread3;
    thread_data td1, td2, td3;

    // ---- Boot OS ----
    initscr();

    WINDOW *heading = newwin(6, 76, 1, 2);
    box(heading, 0, 0);
    mvwprintw(heading, 2, 25, "ULTIMA 2.0 OS - IPC");
    mvwprintw(heading, 4,  2, "Initializing Scheduler...");
    mvwprintw(heading, 5,  2, "Initializing Semaphores...");
    mvwprintw(heading, 6,  2, "Creating Tasks...");

    // ---- Log Window ----
    WINDOW *log_win = create_window(30, 76, 17, 2);
    write_window(log_win, "System Log Started\n");

    // ---- Create Task 1 ----
    td1.thread_id    = 1;
    td1.thread_state = READY;
    td1.sleep_time   = 2;
    td1.kill_signal  = false;
    td1.thread_win   = create_window(8, 24, 8, 2);
    write_window(td1.thread_win, "Starting Task 1\n");

    launch_task(&thread1, &td1);
    scheduler.create_task("Task 1", thread1);
    write_window(log_win, "Task 1 created\n");

    TCB* t1 = scheduler.get_tcb(1);
    t1->allowed_targets      = {1, 2};
    t1->allowed_message_types = {0};

    // ---- Create Task 2 ----
    td2.thread_id    = 2;
    td2.thread_state = READY;
    td2.sleep_time   = 3;
    td2.kill_signal  = false;
    td2.thread_win   = create_window(8, 24, 8, 28);
    write_window(td2.thread_win, "Starting Task 2\n");

    launch_task(&thread2, &td2);
    scheduler.create_task("Task 2", thread2);
    write_window(log_win, "Task 2 created\n");

    TCB* t2 = scheduler.get_tcb(2);
    t2->allowed_targets      = {0, 2};
    t2->allowed_message_types = {0};

    // ---- Create Task 3 ----
    td3.thread_id    = 3;
    td3.thread_state = READY;
    td3.sleep_time   = 4;
    td3.kill_signal  = false;
    td3.thread_win   = create_window(8, 24, 8, 54);
    write_window(td3.thread_win, "Starting Task 3\n");

    launch_task(&thread3, &td3);
    scheduler.create_task("Task 3", thread3);
    write_window(log_win, "Task 3 created\n");

    TCB* t3 = scheduler.get_tcb(3);
    t3->allowed_targets      = {0, 1};
    t3->allowed_message_types = {0};

    // ---- OS Loop ----
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);

    int input;

    while ((input = getch()) != 'q')
    {
        if (input >= '1' && input <= '9') {
            int id = input - '0';
            signal_table.signal_kill(id);

            char msg[64];
            sprintf(msg, "Kill signal -> Task %d\n", id);
            write_window(log_win, msg);
        }

        scheduler.garbage_collect();
        dump_system(log_win);

        sema.down();
        wnoutrefresh(heading);
        wnoutrefresh(log_win);
        wnoutrefresh(td1.thread_win);
        wnoutrefresh(td2.thread_win);
        wnoutrefresh(td3.thread_win);
        doupdate();
        sema.up();

        sleep(1);
    }

    // ---- SHUTDOWN ----
    write_window(log_win, "Shutting down all tasks...\n");

    td1.kill_signal = true;
    td2.kill_signal = true;
    td3.kill_signal = true;

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    write_window(log_win, "All tasks terminated.\n");

    werase(log_win);
    box(log_win, 0, 0);

    scheduler.dump(log_win, 2);
    sema.dump(log_win, 1);

    wrefresh(log_win);

    sleep(3);
    endwin();

    return 0;
}