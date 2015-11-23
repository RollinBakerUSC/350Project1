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
        toBeDeleted = false;
        numWaiters = 0;
    }
    int serverLockIndex;
    List* waitQueue;
    char* name;
    bool toBeDeleted;
    int numWaiters;
};

struct ServerMV {
    ServerMV(int size, char* _name) {
        values = new int[size];
        for(int i = 0; i < size; i++) {
            values[i] = 0;
        }
        name = _name;
        numElements = size;
    }
    int* values;
    char* name;
    int numElements;
};

struct ReplyMsg {
    ReplyMsg(int id, int box, int len, char* msg) {
        idToSendTo = id;
        boxToSendTo = box;
        msgLength = len;
        responseMsg = msg;
    }
    int idToSendTo;
    int boxToSendTo;
    int msgLength;
    char* responseMsg;
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
    cout << "Attempting to Destroy Lock at index " << index << endl;
    if(index > -1 && (unsigned int)index < serverLockTable.size()) {
        ServerLock* toDelete = serverLockTable.at(index);
        if(toDelete != NULL) {
            if(toDelete->numWaiters == 0 && toDelete->available) {
                cout << "Destroying Lock at index " << index << endl;
                delete toDelete;
            } else {
                toDelete->toBeDeleted = true;
            }
        }
    }
}

int Acquire(int index) {
    cout << "Attempting to acquire Lock at index " << index << endl;
    if(index > -1 && (unsigned int)index < serverLockTable.size()) {
        ServerLock* toAcquire = serverLockTable.at(index);
        if(toAcquire != NULL && toAcquire->available) {
            toAcquire->available = false;
            return 0;
        } else {
            return 1;
        }
    } else {
        cout << "Attempted to acquire nonexistant lock" << endl;
        return -1;
    }
}

void Release(int index) {
    cout << "Attempting to Release Lock at index " << index << endl;
    ServerLock* toRelease = serverLockTable.at(index);
    if(toRelease != NULL) {
        cout << "Releasing Lock at index " << index << endl;
        if(toRelease->numWaiters > 0) {
            toRelease->numWaiters--;
            ReplyMsg* newMsg = (ReplyMsg*) toRelease->waitQueue->Remove();
            PacketHeader outPktHdr;
            MailHeader outMailHdr;
            outPktHdr.to = newMsg->idToSendTo;
            toRelease->owner = newMsg->idToSendTo;
            outMailHdr.to = newMsg->boxToSendTo;
            outMailHdr.length = newMsg->msgLength;
            bool success = postOffice->Send(outPktHdr, outMailHdr, newMsg->responseMsg);
            if(!success) {
                cout << "Unable to send response to new owner thread on a Release of Lock at index " << index << endl;
            }
            delete newMsg->responseMsg;
            delete newMsg;
        } else if(toRelease->toBeDeleted) {
            cout << "Destroying Lock at index " << index << " after a Release" << endl;
            delete toRelease;
        } else {
            toRelease->owner = -1;
            toRelease->available = true;
        }
    }
    
}

int CreateCV(char* cvName) {
    cout << "Creating CV with name " << cvName << " at position " << serverCVTable.size() << endl;
    ServerCV* newCV = new ServerCV(cvName);
    serverCVTable.push_back(newCV);
    return serverCVTable.size()-1;
}

void Signal(int condIndex, int lockIndex) {
    cout << "Attempting to signal on CV at index " << condIndex << " with lock at index " << lockIndex << endl;
    if(condIndex > -1 && (unsigned int)condIndex < serverCVTable.size()) {
        if(serverCVTable.at(condIndex) != NULL) {
            ServerCV* toSignal = serverCVTable.at(condIndex);
            if(toSignal->serverLockIndex == lockIndex) {
                if(toSignal->numWaiters > 0) {
                    cout << "Signalling on CV at index " << condIndex << " with lock at index " << lockIndex << endl;
                    toSignal->numWaiters--;
                    ReplyMsg* newMsg = (ReplyMsg*) toSignal->waitQueue->Remove();
                    ServerLock* lock = serverLockTable.at(lockIndex);
                    lock->numWaiters++;
                    lock->waitQueue->Append((void*) newMsg);
                    if(toSignal->numWaiters == 0 && toSignal->toBeDeleted) {
                        cout << "Destroying CV at index " << condIndex << " after a Signal" << endl;
                        delete toSignal;
                    }
                } else if(toSignal->toBeDeleted) {
                    cout << "Destroying CV at index " << condIndex << " after a Signal" << endl;
                    delete toSignal;
                }
            } else {
                cout << "Attempted to signal on CV with wrong lock" << endl;
            }
        }
    }
}

void Broadcast(int condIndex, int lockIndex) {
    cout << "Attempting to broadcast on CV at index " << condIndex << " with lock at index " << lockIndex << endl;
    if(condIndex > -1 && (unsigned int)condIndex < serverCVTable.size()) {
        if(serverCVTable.at(condIndex) != NULL) {
            ServerCV* toSignal = serverCVTable.at(condIndex);
            if(toSignal->serverLockIndex == lockIndex) {
                while(toSignal->numWaiters > 0) {
                    cout << "In Broadcast, Signalling on CV at index " << condIndex << " with lock at index " << lockIndex << endl;
                    toSignal->numWaiters--;
                    ReplyMsg* newMsg = (ReplyMsg*) toSignal->waitQueue->Remove();
                    ServerLock* lock = serverLockTable.at(lockIndex);
                    lock->numWaiters++;
                    lock->waitQueue->Append((void*) newMsg);
                }
                if(toSignal->toBeDeleted) {
                    cout << "Destroying CV at index " << condIndex << " after a Broadcast" << endl;
                    delete toSignal;
                }
            } else {
                cout << "Attempted to Broadcast on CV with wrong lock" << endl;
            }
        }
    }
}

void Wait(int condIndex, int lockIndex) {
    serverCVTable.at(condIndex)->serverLockIndex = lockIndex;
    ServerLock* toRelease = serverLockTable.at(lockIndex);
    cout << "Releasing Lock at index " << lockIndex << " as part of Wait" << endl;
    if(toRelease->numWaiters > 0) {
        toRelease->numWaiters--;
        ReplyMsg* newMsg = (ReplyMsg*) toRelease->waitQueue->Remove();
        PacketHeader outPktHdr;
        MailHeader outMailHdr;
        outPktHdr.to = newMsg->idToSendTo;
        toRelease->owner = newMsg->idToSendTo;
        outMailHdr.to = newMsg->boxToSendTo;
        outMailHdr.length = newMsg->msgLength;
        bool success = postOffice->Send(outPktHdr, outMailHdr, newMsg->responseMsg);
        if(!success) {
            cout << "Unable to send response to new owner thread during Wait on a Release of Lock at index " << lockIndex << endl;
        }
        delete newMsg->responseMsg;
        delete newMsg;
    }
    else {
        toRelease->owner = -1;
        toRelease->available = true;
    }
}

void DestroyCV(int index) {
    cout << "Attempting to Destroy CV at index " << index << endl;
    if(index > -1 && (unsigned int)index < serverCVTable.size()) {
        ServerCV* toDelete = serverCVTable.at(index);
        if(toDelete != NULL) {
            if(toDelete->numWaiters == 0) {
                cout << "Destroying CV at index " << index << endl;
                delete toDelete;
            } else {
                toDelete->toBeDeleted = true;
            }
        }
    }
}

int CreateMV(char* mv_Name, int numElements){
    cout << "Creating MV with name " << mv_Name << " at position " << serverMVTable.size() << endl;
    ServerMV* newMV = new ServerMV(numElements, mv_Name);
    serverMVTable.push_back(newMV);
    return serverMVTable.size() - 1;
}

void SetMV(int index, int arrayIndex, int value){
    cout << "Setting MV number " << index << " at array location " << arrayIndex << "to new value of " << value <<endl;
    if(index > -1 && (unsigned int)index < serverMVTable.size()
        && serverMVTable.at(index) != NULL){
        int arraySize = serverMVTable.at(index)->numElements;
        if(arrayIndex > -1 && arrayIndex < arraySize){
            serverMVTable.at(index)->values[arrayIndex] = value;
        }
    }
}

int GetMV(int index, int arrayIndex){
    cout << "Getting MV number " << index << " at array location " <<arrayIndex<<endl;
    if(index > -1 && (unsigned int)index < serverMVTable.size()
        && serverMVTable.at(index) != NULL){
        int arraySize = serverMVTable.at(index)->numElements;
        if(arrayIndex > -1 && arrayIndex < arraySize){
            return serverMVTable.at(index)->values[arrayIndex];
        }
    }
    return -1;
}

void DestroyMV(int index){
    cout << "Destroying MV number " <<index<<endl;
    if(index > -1 && (unsigned int)index < serverMVTable.size()
        && serverMVTable.at(index) != NULL){
        delete serverMVTable.at(index);
    }
}

void Server() {
    cout << "Starting Nachos Server" << endl;

    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char* request = new char[40];
    char* response = new char[40];
    int machineID, index, length;
    int condIndex, lockIndex;
    bool success;
    char nameLength;
    
    while(true) {
        for(int i = 0; i < 40; i++) {
            request[i] = 0;
        }
        response = new char[40];
        postOffice->Receive(0, &inPktHdr, &inMailHdr, request);
        char opCode = request[0];
        switch(opCode) {
            case 0: // create lock
                nameLength = request[1];
                length = (int)nameLength;
                char* lockName = new char[length+1];
                for(int i = 0; i < length; i++) {
                    lockName[i] = request[i+2];
                }
                lockName[length] = '\0';
                index = CreateLock(lockName);
                response[0] = (char)index;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to CreateLock syscall" << endl;
                }
                break;
            case 1: // acquire lock
                index = (int)request[1];
                int canAcquire = Acquire(index);
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                if(canAcquire == 0) {
                    cout << "Acquiring Lock at index " << index << endl;
                    serverLockTable.at(index)->owner = outPktHdr.to;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    delete response;
                    if(!success) {
                        cout << "Unable to deliver response to Acquire syscall" << endl;
                    }
                } else if(canAcquire == 1){
                    ReplyMsg* reply = new ReplyMsg(outPktHdr.to, outMailHdr.to, outMailHdr.length, response);
                    ServerLock* toAcquire = serverLockTable.at(index);
                    toAcquire->numWaiters++;
                    toAcquire->waitQueue->Append((void*) reply);
                } else {
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    delete response;
                    if(!success) {
                        cout << "Unable to deliver response to Acquire syscall" << endl;
                    }
                }
                break;
            case 2: // release lock
                index = (int)request[1];
                if(index > -1 && (unsigned int)index < serverLockTable.size()) {
                    if(serverLockTable.at(index) != NULL && serverLockTable.at(index)->owner == inPktHdr.from) {
                        Release(index);
                    } else {
                        cout << "Non owner trying to release Lock at index " << index << endl;
                    }
                } else {
                    cout << "Trying to Release a nonexistant lock at index " << index << endl;
                }
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to Release syscall" << endl;
                }
                break;
            case 3: // destroy lock
                index = (int)request[1];
                DestroyLock(index);
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to DestroyLock syscall" << endl;
                }
                break;
            case 4: // create cv
                nameLength = request[1];
                length = (int)nameLength;
                char* cvName = new char[length+1];
                for(int i = 0; i < length; i++) {
                    cvName[i] = request[i+2];
                }
                cvName[length] = '\0';
                index = CreateCV(cvName);
                response[0] = (char)index;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to CreateCV syscall" << endl;
                }
                break;
            case 5: // signal cv
                condIndex = (int)request[1];
                lockIndex = (int)request[2];
                if(lockIndex > -1 && (unsigned int)lockIndex < serverLockTable.size()
                    && serverLockTable.at(lockIndex) != NULL 
                    && serverLockTable.at(lockIndex)->owner == inPktHdr.from) {
                    Signal(condIndex, lockIndex);
                } else {
                    cout << "Trying to signal with bad Lock at index " << index << endl;
                }
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to Signal syscall" << endl;
                }
                break;
            case 6: // broadcast cv
                condIndex = (int)request[1];
                lockIndex = (int)request[2];
                if(lockIndex > -1 && (unsigned int)lockIndex < serverLockTable.size()
                    && serverLockTable.at(lockIndex) != NULL 
                    && serverLockTable.at(lockIndex)->owner == inPktHdr.from) {
                    Broadcast(condIndex, lockIndex);
                } else {
                    cout << "Trying to Broadcast with bad Lock at index " << index << endl;
                }
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to Broadcast syscall" << endl;
                }
                break;
            case 7: // wait cv
                condIndex = (int)request[1];
                lockIndex = (int)request[2];
                if(lockIndex > -1 && (unsigned int)lockIndex < serverLockTable.size()
                    && serverLockTable.at(lockIndex) != NULL 
                    && serverLockTable.at(lockIndex)->owner == inPktHdr.from) {
                    if(condIndex > -1 && (unsigned int)condIndex < serverCVTable.size()
                        && serverCVTable.at(condIndex) != NULL
                        && (serverCVTable.at(condIndex)->serverLockIndex == lockIndex ||
                            serverCVTable.at(condIndex)->serverLockIndex == -1)) {
                        Wait(condIndex, lockIndex);
                        cout << "Waiting on CV at index " << condIndex << " with lock at index " << lockIndex << endl;
                        response[0] = 255;
                        outPktHdr.to = inPktHdr.from;
                        outMailHdr.to = inMailHdr.from;
                        outMailHdr.length = 1;
                        ReplyMsg* reply = new ReplyMsg(outPktHdr.to, outMailHdr.to, outMailHdr.length, response);
                        ServerCV* toWait = serverCVTable.at(index);
                        toWait->numWaiters++;
                        toWait->waitQueue->Append((void*) reply);
                    }
                    else {
                        cout << "Trying to wait on bad condition at index " << condIndex << endl;
                        response[0] = 255;
                        outPktHdr.to = inPktHdr.from;
                        outMailHdr.to = inMailHdr.from;
                        outMailHdr.length = 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, response);
                        delete response;
                        if(!success) {
                            cout << "Unable to deliver response to Wait syscall" << endl;
                        }
                    }   
                } else {
                    cout << "Trying to wait with bad Lock at index " << lockIndex << endl;
                    response[0] = 255;
                    outPktHdr.to = inPktHdr.from;
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.length = 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    delete response;
                    if(!success) {
                        cout << "Unable to deliver response to Wait syscall" << endl;
                    }
                }
                break;
            case 8: // destroy cv
                index = (int)request[1];
                DestroyCV(index);
                response[0] = 255;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                delete response;
                if(!success) {
                    cout << "Unable to deliver response to DestroyLock syscall" << endl;
                }
                break;
            case 9: // create mv
                // Get length of name
                length = (int)request[1];

                // Allocate space for name
                char* mv_Name = new char[length+1];

                // Read name in
                for(int i = 0; i < length; i++) {
                    mv_Name[i] = request[i+2];
                }
                mv_Name[length] = '\0';

                // Get number of elements
                int numElements = (int)request[length + 2];

                index = CreateMV(mv_Name, numElements);

                response[0] = (char)index;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to CreateMV syscall" << endl;
                }
                break;
            case 10: // set mv
                index = (int)request[1];
                int arrayIndex = (int)request[2];
                int value = (int)request[3];
                SetMV(index, arrayIndex, value);

                response[0] = 127;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to SetMV syscall" << endl;
                }
                break;
            case 11: // get mv
                index = (int)request[1];
                int mv_arrayIndex = (int)request[2];

                int returnval = GetMV(index, mv_arrayIndex);

                response[0] = (char) returnval;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to GetMV syscall" << endl;
                }
                break;
            case 12: // destroy mv
                index = (int)request[1];
                DestroyMV(index);
                response[0] = 127;
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = inMailHdr.from;
                outMailHdr.length = 1;
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                if(!success) {
                    cout << "Unable to deliver response to DestroyMV syscall" << endl;
                }
                break;
        }
    }
}
