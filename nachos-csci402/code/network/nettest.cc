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
    ServerLock(char* _name, unsigned int _id) {
        owner = -1;
        available = true;
        ownerMailBoxNum = -1;
        waitQueue = new List();
        name = _name;
        numWaiters = 0;
        toBeDeleted = false;
        id = _id;
    }
    int owner;
    unsigned int id;
    bool available;
    int ownerMailBoxNum;
    List* waitQueue;
    char* name;
    int numWaiters;
    bool toBeDeleted;
};

struct ServerCV {
    ServerCV(char* _name, unsigned int _id) {
        serverLockIndex = -1;
        waitQueue = new List();
        name = _name;
        toBeDeleted = false;
        numWaiters = 0;
        id = _id;
    }
    int serverLockIndex;
    unsigned int id;
    List* waitQueue;
    char* name;
    bool toBeDeleted;
    int numWaiters;
};

struct ServerMV {
    ServerMV(int size, char* _name, unsigned int _id) {
        values = new int[size];
        for(int i = 0; i < size; i++) {
            values[i] = 0;
        }
        name = _name;
        numElements = size;
        id = _id;
    }
    int* values;
    unsigned int id;
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

struct PendingRequest {
    PendingRequest(char _opCode, int _clientID, int _clientMail, char* _name) {
        noCount = 0;
        opCode = _opCode;
        clientID = _clientID;
        clientMail = _clientMail;
        name = _name;
    }
    int noCount;
    char opCode;
    int clientID;
    int clientMail;
    char* name;
};

std::vector<ServerLock*> serverLockTable;
std::vector<ServerCV*> serverCVTable;
std::vector<ServerMV*> serverMVTable;
std::vector<PendingRequest*> pendingRequestTable;


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

int updatePendingRequestTable(int requestID) {
    pendingRequestTable[requestID]->noCount += 1;
    return pendingRequestTable[requestID]->noCount;
}

int CreateLock(char* lockName) {
    unsigned int uniqueID = serverLockTable.size()+serverID*50;
    cout << "Creating Lock with name " << lockName << " with unique id " << uniqueID << " at position " << serverLockTable.size() << endl;
    ServerLock* newLock = new ServerLock(lockName, uniqueID);
    serverLockTable.push_back(newLock);
    return (int)uniqueID;
}

int getLockFromTable(char* lockName) {
    for(unsigned int i = 0; i < serverLockTable.size(); i++) {
        if(strcmp(lockName, serverLockTable[i]->name) == 0) {
            cout << "Lock with name " << lockName << " already created at position " << i << endl;
            return (int)serverLockTable[i]->id;
        }
    }
    return -1;
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
    for(unsigned int i = 0; i < serverCVTable.size(); i++) {
        if(strcmp(cvName, serverCVTable[i]->name) == 0) {
            cout << "CV with name " << cvName << " already created at position " << i << endl;
            return i;
        }
    }
    cout << "Creating CV with name " << cvName << " at position " << serverCVTable.size() << endl;
    unsigned int uniqueID = serverCVTable.size()+serverID*50;
    ServerCV* newCV = new ServerCV(cvName, uniqueID);
    serverCVTable.push_back(newCV);
    return uniqueID;
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

int CreateMV(char* mv_Name, int numElements) {
    for(unsigned int i = 0; i < serverMVTable.size(); i++) {
        if(strcmp(mv_Name, serverMVTable[i]->name) == 0) {
            cout << "MV with name " << mv_Name << " already created at position " << i << endl;
            return i;
        }
    }
    cout << "Creating MV with name " << mv_Name << " at position " << serverMVTable.size() << endl;
    unsigned int uniqueID = serverMVTable.size()+serverID*50;
    ServerMV* newMV = new ServerMV(numElements, mv_Name, uniqueID);
    serverMVTable.push_back(newMV);
    return uniqueID;
}

void SetMV(int index, int arrayIndex, int value){
    cout << "Setting MV number " << index << " at array location " << arrayIndex << " to new value of " << value <<endl;
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

    PendingRequest* newPendingRequest;
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char* request = new char[40];
    char* response = new char[40];
    int machineID, index, length;
    int condIndex, lockIndex;
    bool success;
    char nameLength;
    int requestingClientID, requestingClientMail;
    int responseID;
    
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
                // check if the lock already exists in our table
                index = getLockFromTable(lockName);
                if(index != -1) {
                    response[0] = (char)index;
                    outPktHdr.to = inPktHdr.from;
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.length = 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                } else if(numServers == 1) { // if we are the only server we just create the lock
                    cout << "Creating lock as only server" << endl;
                    index = CreateLock(lockName);
                    response[0] = (char)index;
                    outPktHdr.to = inPktHdr.from;
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.length = 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                } else { // if there are other servers we have to ask if they have the lock
                    response[0] = 13; // opcode for server 
                    response[1] = (char)inPktHdr.from; // client machine id
                    response[2] = (char)inMailHdr.from; // client mailbox
                    response[3] = nameLength;
                    response[4] = (char)pendingRequestTable.size();
                    newPendingRequest = new PendingRequest(opCode, inPktHdr.from, inMailHdr.from, lockName);
                    pendingRequestTable.push_back(newPendingRequest);
                    for(int i = 0; i < length; i++) {
                        response[5+i] = lockName[i];
                    }
                    for(int i = 0; i < numServers; i++) {
                        if(i != serverID) {
                            outPktHdr.to = i;
                            outMailHdr.to = 0;
                            outMailHdr.from = 0;
                            outMailHdr.length = 5+length;
                            success = postOffice->Send(outPktHdr, outMailHdr, response);
                            cout << "Asking server " << i << " if they have lock" << endl;
                            if(!success) {
                                cout << "Unable to forward CreateLock syscall" << endl;
                            }
                        }
                    }
                }
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
                    cout << "Trying to signal with bad Lock at index " << lockIndex << endl;
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
                    cout << "Trying to Broadcast with bad Lock at index " << lockIndex << endl;
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
                        ServerCV* toWait = serverCVTable.at(condIndex);
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
            case 13: // server create lock request
                requestingClientID = (int)request[1];
                requestingClientMail = (int)request[2];
                length = (int)request[3];
                responseID = (int)request[4];
                char* locksName = new char[length+1];
                for(int i = 0; i < length; i++) {
                    locksName[i] = request[5+i];
                }
                locksName[length] = '\0';
                index = getLockFromTable(locksName);
                if(index != -1) { // we have the lock the client wants to create
                    response[0] = 14; // op code for responding to a create lock request
                    response[1] = 1;
                    response[2] = (char)responseID;
                    outPktHdr.to = inPktHdr.from;
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.length = 3;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    if(!success) {
                        cout << "Unable to deliver response to server create lock" << endl;
                    }
                    response[0] = (char)index;
                    outPktHdr.to = requestingClientID;
                    outMailHdr.to = requestingClientMail;
                    outMailHdr.length = 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    if(!success) {
                        cout << "Unable to deliver response to client create lock" << endl;
                    }
                } else { // we do not have the lock the client wants to create
                    response[0] = 14; // op code for responding to a create lock request
                    response[1] = 0;
                    response[2] = (char)responseID;
                    outPktHdr.to = inPktHdr.from;
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.length = 3;
                    cout << "Telling server " << outPktHdr.to << " that we do not have lock" << endl;
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    if(!success) {
                        cout << "Unable to deliver response to server create lock" << endl;
                    }
                }
                break;
            case 14: // response from server create lock
                if(request[1] == 0) {
                    responseID = (int)response[2];
                    int numNos = updatePendingRequestTable(responseID);
                    if(numNos == numServers-1) {
                        PendingRequest* pr = pendingRequestTable[responseID];
                        index = CreateLock(pr->name);
                        response[0] = (char)index;
                        outPktHdr.to = pr->clientID;
                        outMailHdr.to = pr->clientMail;
                        outMailHdr.length = 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, response);
                        if(!success) {
                            cout << "Unable to deliver response to client create lock" << endl;
                        }
                    }

                }
                break;
        }
    }
}
