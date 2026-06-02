/*=============================================================
 * FILE:    IPC.h
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
 *    and receive a message meant for another. A
 *=============================================================*/


#ifndef IPC_H
#define IPC_H

#include <ctime>
#include <queue>
#include <string>
#include <ncurses.h>
#include <cstdint>				//added for security reasons
#include <vector>				//added for security reasons
#include "Sema.h"

class ipc {
	public:
	
	struct Message_Type {
		int Message_Type_Id;
		char Message_Type_Description[64];
	};
	
	struct Message {
		int Source_Task_Id;
		int Destination_Task_Id;
		time_t Message_Arrival_Time;
		Message_Type Msg_Type;
		int Msg_Size;
		char *Msg_Text;
		
		//added both of these to each message in order to flag approved pathing & verifiable integrity value
		bool authenticated;		//added for security reasons
		uint32_t checksum;		//added for security reasons
	};
	
	//Constructor: Initializes the mailbox for max_tasks, Return -1 on error.
	ipc(int max_tasks);
	
	//Destructor: Uses Message_DeleteAll to ensure all messages are deleted from memory.
	~ipc();
	
	//Struct based: For forwarding or system-level precision
	int Message_Send(Message *msg);
	
	//Overloaded: Shortcut for standard tasks
	int Message_Send(int S_Id, int D_Id, char *Mess, int Mess_Type);
	
	//Struct based: Populates exisiting struct with the next message
	int Message_Receive(int Task_Id, Message *msg);
	
	//Populates an exisiting struct with next message : altered to allow receive.
	int Message_Receive(int Task_Id, char *Mess, int buffer_size, int *Mess_Type);
	
	//Number of messages for a specific task
	int Message_Count(int Task_Id);
	
	//Total number of messages across the entire OS
	int Message_Count();
	
	//Print messages for one task without removing them
	void Message_Print(WINDOW *win, int Task_id, int &row);
	
	//Wipe the queue for a specific task
	int Message_DeleteAll(int Task_id);
	
	//System wide debug dump of all messages
	void ipc_Message_Dump(WINDOW *win, int &row);
	
	private:
	
	int max_tasks;
	
	bool has_send_permission(int source, int dest, int msg_type);		//added for security reasons
	
	std::vector<std::queue<Message*>> mailboxes;
	std::vector<Semaphore> mailbox_locks;
	
	bool isValidId(int id);
	
	uint32_t compute_checksum(const char *text);
    bool verify_checksum(const Message *msg);
	bool has_receive_permission(int receiver, const Message *msg);
};

#endif