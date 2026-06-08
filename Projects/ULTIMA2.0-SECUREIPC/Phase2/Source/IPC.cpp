/*=============================================================
 * FILE:    IPC.cpp
 * PROJECT: ULTIMA 2.0 - Phase 2 (Message Passing)
 * AUTHOR:  Alvaro Espinoza
 * DATE:    Spring 2026
 *
 * DESCRIPTION:
 *   Class declaration of the IPC.
 *
 *   KEY DESIGN DECISION – Message Passage:
 *     Functions will handle message delivery with a Message_Send()
 *     and Message_Receive(). Each task is to have its own mailbox
 *     that contains messages that are received and sent.
 *
 *   INTEGRATION WITH TCB:
 *     A mailbox is a queue of messages that it accessible through
 *     the task's TCB. Each message is to be protected with the use
 *    of a binary semaphore to ensure another task does not interfere
 *    and receive a message meant for another.
 *=============================================================*/
#include <cstring>
#include <iostream>
#include <ctime>
#include <vector>
#include <cstdint>			//added for security reasons
#include <queue>
#include <string>
#include "IPC.h"
#include "Sched.h"

constexpr size_t MAX_MSG_LEN = 255;		//added for size limit security reasons.

using namespace std;


/*====================================
 * Constructor
/* ===================================*/
ipc::ipc(int max_task) {
	
    if (max_task <= 0) {
        max_tasks = 0;
        return;
    }

    max_tasks = max_task;
	
    mailboxes.resize(max_tasks);
	mailbox_locks.resize(max_tasks);
}

/*====================================
* Destructor
* Ensure that pointers to messages in each mailbox 
* are deleted when the OS terminates.
/* ===================================*/
ipc::~ipc() {
	
	for(int i = 0; i < max_tasks; i++) {
		Message_DeleteAll(i);
	}
}
/*====================================
* Security Permissions
* Ensures that only allowed targets are able
* to access/manipulate
*=====================================*/
//Added sender-sided authorization
bool ipc::has_send_permission(int source, int dest, int msg_type) {
	TCB* src_tcb = scheduler.get_tcb(source + 1);		// +1 because ID starts at 1
	if (!src_tcb) return false;
	
	// Check allowed targets
	bool allowed_target = false;
	for (int t : src_tcb->allowed_targets) {
		if (t == dest) {
			allowed_target = true;
			break;
		}
	}
	
	if (!allowed_target) return false;
	
	//Check allowed message types
	bool allowed_type = false;
	for (int m : src_tcb->allowed_message_types) {
		if (m == msg_type) {
			allowed_type = true;
			break;
		}
	}
	return allowed_type;
}

//Added receiver-sided authorization
bool ipc::has_receive_permission(int receiver, const Message *msg) {
    if (!msg) return false;

    TCB* recv_tcb = scheduler.get_tcb(receiver + 1);   //IPC uses 0-based IDs
    if (!recv_tcb) return false;

    bool allowed_source = false;
    for (int t : recv_tcb->allowed_targets) {
        if (t == msg->Source_Task_Id) {
            allowed_source = true;
            break;
        }
    }

    if (!allowed_source) return false;

    bool allowed_type = false;
    for (int m : recv_tcb->allowed_message_types) {
        if (m == msg->Msg_Type.Message_Type_Id) {
            allowed_type = true;
            break;
        }
    }

    return allowed_type;
}

	uint32_t ipc::compute_checksum(const char *text) {
    if (!text) return 0;

    uint32_t sum = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        sum += (unsigned char)text[i];
    }
    return sum;
}

//On receive, we verify checksum before accepting the message : message-integrity checking.
bool ipc::verify_checksum(const Message *msg) {
    if (!msg || !msg->Msg_Text) return false;
    return msg->checksum == compute_checksum(msg->Msg_Text);
}
	
/*====================================
 * Validate the Task ID
/* ===================================*/
bool ipc::isValidId(int id) {
	return (id >= 0 && id < max_tasks);
}

/*====================================
 * Message_Send (for structs)
 * previously, this struct could bypass the secure checks,
 * we routed it through the secure overloaded send.
/* ===================================*/
int ipc::Message_Send(Message *msg) {
    if (!msg || !msg->Msg_Text)
        return -1;

    return Message_Send(
        msg->Source_Task_Id,
        msg->Destination_Task_Id,
        msg->Msg_Text,
        msg->Msg_Type.Message_Type_Id
    );
}

/*====================================
 * Message_Send (overloaded)
/* ===================================*/

int ipc::Message_Send(int S_Id, int D_Id, char *Mess, int Mess_Type) {
    if (!isValidId(S_Id) || !isValidId(D_Id) || !Mess)		//added for security reasons
        return -1;											//bad arguments/invalid id
	
	if (!has_send_permission(S_Id, D_Id, Mess_Type))		//added for security reasons
		return -2;											//sender is not allowed

    if (strlen(Mess) > MAX_MSG_LEN)							//added for security reasons
        return -3;											//message too large
		
	int state = scheduler.get_state(D_Id + 1);
	
    if (state == TERMINATED || state == -1)
        return 0;

    Message *msg = new Message;

    msg->Source_Task_Id = S_Id;
    msg->Destination_Task_Id = D_Id;
    msg->Message_Arrival_Time = time(nullptr);
    msg->Msg_Type.Message_Type_Id = Mess_Type;

    msg->Msg_Size = strlen(Mess) + 1;
    msg->Msg_Text = new char[msg->Msg_Size];
    strcpy(msg->Msg_Text, Mess);
	

	
	msg->checksum = compute_checksum(msg->Msg_Text);		//added for security reasons
    msg->authenticated = true;								//added for security reasons : mark message as authenticated

    mailbox_locks[D_Id].down();

if (state == TERMINATED || state == -1) {
    mailbox_locks[D_Id].up();

    if (msg->Msg_Text) {
        memset(msg->Msg_Text, 0, msg->Msg_Size);		//failed send cleanup
        delete[] msg->Msg_Text;
    }
    delete msg;

    return 0;
}

    mailboxes[D_Id].push(msg);

    mailbox_locks[D_Id].up();

    return 1;
}	

/*====================================
 * Struct version of Message_Receive
/* ===================================*/
int ipc::Message_Receive(int Task_Id, Message *msg) {
		if (!isValidId(Task_Id) || !msg)
		return -1;
	
	mailbox_locks[Task_Id].down();
	
	if (mailboxes[Task_Id].empty()) {
		mailbox_locks[Task_Id].up();
		return 0;
	}
	
	Message *front = mailboxes[Task_Id].front();
	mailboxes[Task_Id].pop();
	
if (!front->authenticated || !verify_checksum(front) ||
    !has_receive_permission(Task_Id, front)) {
    if (front->Msg_Text) {
        memset(front->Msg_Text, 0, front->Msg_Size);	//bad message cleanup
        delete[] front->Msg_Text;
    }
    delete front;
    mailbox_locks[Task_Id].up();
    return -2;
}
	
	mailbox_locks[Task_Id].up();
	
	*msg = *front;	
	delete front;
	
	return 1;
}

/*====================================
 * Message_Receive (overloaded)
/* ===================================*/
int ipc::Message_Receive(int Task_Id, char *Mess, int buffer_size, int *Mess_Type) {
    if (!isValidId(Task_Id) || !Mess || !Mess_Type || buffer_size <= 0)
        return -1;

    Message temp;

    int result = Message_Receive(Task_Id, &temp);
    if (result <= 0)
        return result;

    strncpy(Mess, temp.Msg_Text, buffer_size - 1);
    Mess[buffer_size - 1] = '\0';
    *Mess_Type = temp.Msg_Type.Message_Type_Id;

    if (temp.Msg_Text) {
        memset(temp.Msg_Text, 0, temp.Msg_Size);		//reduces residual message
        delete[] temp.Msg_Text;
    }

    return 1;
}

/*====================================
 * Counting the number of messages per task.
/* ===================================*/

int ipc::Message_Count(int Task_Id) {
	
	if (!isValidId(Task_Id))
		return -1;
	
	mailbox_locks[Task_Id].down();
	
	int count = mailboxes[Task_Id].size();
	
	mailbox_locks[Task_Id].up();
	
	return count;
}

/*====================================
 * Counting total number of messages in OS
/* ===================================*/

int ipc::Message_Count() {
	
	int total = 0;
	
	for(int i = 0; i < max_tasks; i++) {
		mailbox_locks[i].down();
		total += mailboxes[i].size();
		mailbox_locks[i].up();
	}
	
	return total;
}

/*====================================
 * Printing out messages
/* ===================================*/
void ipc::Message_Print(WINDOW *win, int Task_Id, int &row) {
    if (!isValidId(Task_Id) || !win)
        return;

    mailbox_locks[Task_Id].down();

    queue<Message*> temp = mailboxes[Task_Id];

    const int MAX_MESSAGES = 3;
    int printed = 0;

    mvwprintw(win, row++, 2, "--- Mailbox Task %d ---", Task_Id + 1);

    while (!temp.empty() && printed < MAX_MESSAGES) {
        Message *msg = temp.front();

        mvwprintw(win, row++, 4,
            "From:%d | %s | Type:%d",
            msg->Source_Task_Id + 1,
            msg->Msg_Text,
            msg->Msg_Type.Message_Type_Id
        );

        temp.pop();
        printed++;
    }

    if (!temp.empty()) {
        mvwprintw(win, row++, 4, "... (%lu more messages)", temp.size());
    }

    mailbox_locks[Task_Id].up();
}

/*====================================
 * Delete all messages in mailbox
/* ===================================*/
int ipc::Message_DeleteAll(int Task_Id) {
	
	if (!isValidId(Task_Id))
		return -1;
	
	mailbox_locks[Task_Id].down();
	
	int count = 0;
	
	while(!mailboxes[Task_Id].empty()) {
		Message *msg = mailboxes[Task_Id].front();
		
		if (msg->Msg_Text) {
			memset(msg->Msg_Text, 0, msg->Msg_Size);	//reduced residual message data
			delete[] msg->Msg_Text;
}
		
		delete msg;
		mailboxes[Task_Id].pop();
		
		count++;
	}
	
	mailbox_locks[Task_Id].up();
	
	return count;
}

/*====================================
 * Display dump of messages per task
/* ===================================*/
void ipc::ipc_Message_Dump(WINDOW *win, int &row) {
	
    if (!win) return;

    mvwprintw(win, row++, 2, "----- IPC Message Dump -----");

    for (int i = 0; i < max_tasks; i++) {
        Message_Print(win, i, row);

        row++;
    }

    mvwprintw(win, row++, 2, "----------------------------");
}