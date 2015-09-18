// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "customer.h"
#include "clerk.h"
#include "applicationclerk.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t4_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif

// global and shared data for Part2
int numCustomers; // the number of customers in the current simulation
int numAppClerks; // the number of application clerks

Customer** customer; // the array of customers - used in CustomerStart
					 // initialized in Part2()

ApplicationClerk** appClerk; // the array of application clerks

Lock* clerkLineLock; // the lock used for a customer to select a line

struct CustomerData {
	bool social;
	bool photos;
};

CustomerData* customerData;

// end of global and shared data for Part2

Customer::Customer(int _socialSecurity, char* _name) : socialSecurity(_socialSecurity), name(_name) {
	int random = rand() % 4;
	money = 100 + random * 500;
}

char* Customer::getName() {
	return name;
}

void Customer::Run() {
	printf("In %s\n", name);
	// first acquire the lock so we know the line won't change
	// after we select it but before we get in line
	clerkLineLock->Acquire();
	int myLine = -1; // will be the index of the clerk whose line is chosen
	int lineSize = numCustomers+1; // the max customers that could be in any line
	for(int i = 0; i < numAppClerks; i++) {
		if(appClerk[i]->getLineCount() < lineSize && // if the line is shorter
		appClerk[i]->getState() != CLERK_BREAK) { // and the clerk is not on break
			myLine = i;
			lineSize = appClerk[i]->getLineCount();
		}
	}
	printf("%s has chosen line %d\n", name, myLine);
	//wait in line
	appClerk[myLine]->incrementLine(); // increase line size
	printf("%s is waiting in line %d\n", name, myLine);
	appClerk[myLine]->waitOnLineCV(); // sleep until the clerk wakes me up
	appClerk[myLine]->decrementLine(); // i have been called up to clerk so line goes down
	
	// now I am with the clerk
	printf("%s is with Clerk %d\n", name, myLine);
	appClerk[myLine]->setState(CLERK_BUSY); // ensure that the clerk is busy
	appClerk[myLine]->acquireLock(); // receive the lock so interaction with clerk is atomic
	clerkLineLock->Release(); // allow other customers to get in line
	// give my data to clerk
	customerData[socialSecurity].social = true; // so the office knows i have given my social
	printf("%s giving Clerk %d his social\n", name, myLine);
	appClerk[myLine]->signalOnClerkCV(); // tell clerk I have given them my social
	appClerk[myLine]->waitOnClerkCV(); // wait until clerk has "filed" my social
	printf("%s is leaving Clerk %d\n", name, myLine);
	appClerk[myLine]->setState(CLERK_FREE); // ensure that the clerk is now free
	appClerk[myLine]->signalOnClerkCV(); // tell clerk I am leaving
	appClerk[myLine]->releaseLock(); // end of the interaction with the clerk
}

Clerk::Clerk(int _id, char* _name) : id(_id), name(_name), state(CLERK_FREE) {
	char* buff = new char[50];
	strcat(buff, _name);
	strcat(buff, " Line Condition");
	clerkLineCV = new Condition(buff);
	buff = new char[50];
	strcat(buff, _name);
	strcat(buff, " Lock");
	clerkLock = new Lock(buff);
	buff = new char[50];
	strcat(buff, _name);
	strcat(buff, " Condition");
	clerkCV = new Condition(buff);
}

char* Clerk::getName() {
	return name;
}

int Clerk::getLineCount() {
	return lineCount;
}

int Clerk::getBribeLineCount() {
	return bribeLineCount;
}

ClerkState Clerk::getState() {
	return state;
}

void Clerk::setState(ClerkState _state) {
	state = _state;
}

void Clerk::incrementLine() {
	lineCount++;
}

void Clerk::decrementLine() {
	lineCount--;
}

void Clerk::incrementBribeLine() {
	bribeLineCount++;
}

void Clerk::decrementBribeLine() {
	bribeLineCount--;
}

void Clerk::waitOnLineCV() {
	clerkLineCV->Wait(clerkLineLock);
}

void Clerk::signalOnLineCV() {
	clerkLineCV->Signal(clerkLineLock);
}

void Clerk::waitOnBribeLineCV() {
	clerkBribeLineCV->Wait(clerkLineLock);
}

void Clerk::signalOnBribeLineCV() {
	clerkBribeLineCV->Signal(clerkLineLock);
}

void Clerk::waitOnClerkCV() {
	clerkCV->Wait(clerkLock);
}

void Clerk::signalOnClerkCV() {
	clerkCV->Signal(clerkLock);
}

void Clerk::acquireLock() {
	clerkLock->Acquire();
}

void Clerk::releaseLock() {
	clerkLock->Release();
}

ApplicationClerk::ApplicationClerk(int _id, char* _name) : Clerk(_id, _name) {
}

void ApplicationClerk::Run() {
	printf("In %s\n", getName());

	while(true) {
		clerkLineLock->Acquire();
		if(getLineCount()>0) { // if there is someone in line
			signalOnLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
		} else {
			setState(CLERK_FREE); // if no one is in line clerk is free
		}
		if (getState() == CLERK_BUSY) { // if clerk is busy then we interact with customer
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			waitOnClerkCV(); // wait for customer to give me social
			// customer has now given social
			printf("%s is filing customer social\n", getName());
			for (int i = 0; i < 10; i++) { // make filing the social take some time
				currentThread->Yield();
			}
			printf("%s has filed customer social\n", getName());
			signalOnClerkCV(); // tell customer I have filed
			waitOnClerkCV(); // wait on Customer to leave
			// once customer has left the interaction is over
			releaseLock();
		} else { // there is no customer to interact with
			clerkLineLock->Release(); // so allow customers to get in line
			currentThread->Yield(); // yield the cpu
		}
		if (getLineCount() == 0 && getState() == CLERK_FREE) { // only here to end the thread
			break; // will be replaced when manager is created
		}
	}
}


// following funtions are used to with Thread->Fork() to begin the clerks and customers processes
// e.g. the function used to kick off a customer thread
void CustomerStart(int index) { // index decides which customer we are starting
	customer[index]->Run();
}

void AppClerkStart(int index) {
	appClerk[index]->Run();
}

void Part2() {
	numCustomers = 4;
	numAppClerks = 2;
	customerData = new CustomerData[numCustomers];
	customer = new Customer*[numCustomers];
	appClerk = new ApplicationClerk*[numAppClerks];
	clerkLineLock = new Lock("Clerk Line Lock");

	char* name;
	for(int i = 0; i < numCustomers; i++) {
		name = new char[10];
		sprintf(name, "Customer %d", i);
		customer[i] = new Customer(i, name);
	}
	for(int i = 0; i < numAppClerks; i++) {
		name = new char[10];
		sprintf(name, "Application Clerk %d", i);
		appClerk[i] = new ApplicationClerk(i, name);
	}

	Thread* t;
	for (int i = 0; i < numCustomers; i++) {
		t = new Thread(customer[i]->getName());
		t->Fork((VoidFunctionPtr)CustomerStart, i);
	}
	for (int i = 0; i < numAppClerks; i++) {
		t = new Thread(appClerk[i]->getName());
		t->Fork((VoidFunctionPtr)AppClerkStart, i);
	}
}
