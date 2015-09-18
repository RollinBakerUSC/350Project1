// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
	name = debugName;
	state = FREE;
	owner = NULL;
	waitQueue = new List;
}

Lock::~Lock() {
	delete waitQueue;
}

void Lock::Acquire() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
	if(isHeldByCurrentThread()) { // in this case nothing needs to be done
		(void) interrupt->SetLevel(oldLevel); //reset interrupts
		return;
	}
	if(state == FREE) { // the thread can acquire the lock
		state = BUSY; // change the state so no other threads can acquire the lock
		owner = currentThread; // update the owner
	} else { // the lock is not available
		waitQueue->Append((void*) currentThread); // place on the waitlist to acquire the lock
		currentThread->Sleep(); // put the thread to sleep so it is not busy waiting
	}
	(void) interrupt->SetLevel(oldLevel); //reset the interrupts
}

void Lock::Release() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
	// only the Thread that owns the lock will be allowed to release
	if(!isHeldByCurrentThread()) {
		printf("Non owner thread, %s, attempted to release lock %s\n",
			 currentThread->getName(), this->getName()); // for debugging
		(void) interrupt->SetLevel(oldLevel); //reset the interrupts
		return;
	}
	// if the waitQueue is not empty we remove the thread from the top of it and
	// give it the lock and finally put it ready to run
	if(!waitQueue->IsEmpty()) {
		Thread *newOwner = (Thread *) waitQueue->Remove(); // take the new thread from the queue
		owner = newOwner; // make it the owner
		scheduler->ReadyToRun(newOwner); // allow the thread to run
	} else { // the queue is empty so no threads are waiting to acquire the lock
		state = FREE;
		owner = NULL;
	}
	(void) interrupt->SetLevel(oldLevel); //reset the interrupts
}

bool Lock::isHeldByCurrentThread() { //returns true if lock is held by current thread
	return (owner == currentThread);
}

Condition::Condition(char* debugName) {
	name = debugName;
	waitingLock = NULL;
	waitQueue = new List;
}

Condition::~Condition() {
	delete waitQueue;
}

void Condition::Wait(Lock* conditionLock) { 
//ASSERT(FALSE);
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
	if(conditionLock == NULL) {
		printf("Attempted to wait on uninitialized lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	if(waitingLock == NULL) { // no one is waiting on this condition variable yet
		waitingLock = conditionLock;
	} else if(conditionLock != waitingLock) {
		printf("Attempted to wait on different lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	waitQueue->Append((void*) currentThread); // place the current thread in the waiting list
	conditionLock->Release(); // release the lock while waiting on the condition
	currentThread->Sleep();
	conditionLock->Acquire(); // reobtain the lock once signaled and woken
	
	(void) interrupt->SetLevel(oldLevel); // reset interrupts
}

void Condition::Signal(Lock* conditionLock) {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
	if(conditionLock == NULL) {
		printf("Attempted to signal without lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	if(waitQueue->IsEmpty()) { // no threads to signal
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	if(conditionLock != waitingLock) {
		printf("Attempted to signal on different lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	// okay to signal now
	Thread *threadToWake = (Thread *) waitQueue->Remove(); // prepare the thread to signal
	scheduler->ReadyToRun(threadToWake); // wake the thread
	if(waitQueue->IsEmpty()) {
		waitingLock = NULL; // no one is waiting on this lock
	}
	(void) interrupt->SetLevel(oldLevel); // reset interrupts
}

void Condition::Broadcast(Lock* conditionLock) {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
	if(conditionLock == NULL) {
		printf("Attempted to broadcast without lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	if(conditionLock != waitingLock) {
		printf("Attempted to broadcast on different lock\n"); // for debugging
		(void) interrupt->SetLevel(oldLevel); // reset interrupts
		return;
	}
	(void) interrupt->SetLevel(oldLevel); // reset interrupts
	while(!waitQueue->IsEmpty()) {
		Signal(conditionLock);
	}
}
