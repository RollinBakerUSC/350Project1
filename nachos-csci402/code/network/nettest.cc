// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message
struct ServerLock {
    ServerLock(char* _name) {
        owner = -1;
        available = true;
        ownerMailBoxNum = -1;
        waitQueue = new List();
        name = _name;
        numWaiters = 0;
        toBeDeleted = false;
    }
    int owner;
    bool available;
    int ownerMailBoxNum;
    List* waitQueue;
    char* name;
    int numWaiters;
    bool toBeDeleted;
};

struct ServerCV {
    ServerCV(char* _name) {
        serverLockIndex = -1;
        waitQueue = new List();
        name = _name;
    }
    int serverLockIndex;
    List* waitQueue;
    char* name;
};

struct ServerMV {
    ServerMV(int size) {
        values = new int[size];
        for(int i = 0; i < size; i++) {
            values[i] = 0;
        }
    }
    int* values;
};

std::vector<ServerLock*> serverLockTable;
std::vector<ServerCV*> serverCVTable;
std::vector<ServerMV*> serverMVTable;

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

int CreateLock(char* lockName) {
    cout << "Creating Lock with name " << lockName << " at position " << serverLockTable.size() << endl;
    ServerLock* newLock = new ServerLock(lockName);
    serverLockTable.push_back(newLock);
    return serverLockTable.size()-1;
}

void DestroyLock(int index) {
    cout << "Destroy Lock at index " << index << endl;
    if(index > -1 && (unsigned int)index < serverLockTable.size()) {
        ServerLock* toDelete = serverLockTable.at(index);
        if(toDelete->numWaiters = 0 && toDelete->available) {
            delete toDelete;
        } else {
            toDelete->toBeDeleted = true;
        }
    }
}

void Server() {
    cout << "Starting Nachos Server" << endl;

    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char* request = new char[40];
    char* response = new char[40];
    int machineID, index;
    bool success;
    
    while(true) {
        for(int i = 0; i < 40; i++) {
            request[i] = 0;
            response[i] = 0;
        }
        postOffice->Receive(0, &inPktHdr, &inMailHdr, request);
        char opCode = request[0];
        switch(opCode) {
            case 0:
                char nameLength = request[1];
                int length = (int)nameLength;
                char* lockName = new char[length+1];
                for(int i = 0; i < nameLength; i++) {
                    lockName[i] = request[i+2];
                }
                lockName[length] = '\0';
                index = CreateLock(lockName);
                response[0] = (char)index;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to CreateLock syscall" << endl;
                }
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                index = (int)request[1];
                DestroyLock(index);
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to CreateLock syscall" << endl;
                }
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
            case 10:
                break;
            case 11:
                break;
            case 12:
                break;
        }
    }
}
