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
#include "pictureclerk.h"
#include "passportclerk.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------
void system_test();

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
Lock* SystemLock; // so the menu won't pop up in the middle of system test
Condition* SystemCondition;

int numCustomers; // the number of customers in the current simulation
int numAppClerks; // the number of application clerks
int numPicClerks; // the number of picture clerks
int numPassClerks; // the number of passport clerks
int numCashiers; // the number of cashiers
int numSenators; // the number of senators

Customer** customer; // the array of customers - used in CustomerStart
					 // initialized in Part2()

ApplicationClerk** appClerk; // the array of application clerks

PictureClerk** picClerk; // the array of picture clerks

PassportClerk** passClerk; // the array of passport clerks

Lock* clerkLineLock; // the lock used for a customer to select a line

struct CustomerData {
	bool arrived; // if the customer has shown up to the office
	bool social;
	bool picture;
	bool passport;
	CustomerData() {
		arrived = false;
		social = false;
		picture = false;
		passport = false;
	}
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
	customerData[socialSecurity].arrived = true;
	int random = rand() % 2; // to decide if we go pic or app first
	if (random == 0) {
		goToAppClerk();
		goToPicClerk();
	} else {
		goToPicClerk();
		goToAppClerk();
	}
	goToPassClerk();
}

void Customer::goToAppClerk() {
	// first we acquire the line lock
	clerkLineLock->Acquire();
	int myLine = -1; // will be the index of the clerk whose line is chosen
	int lineSize = numCustomers + 1; // the max customers that could be in any line
	bool hasBribed = false;
	for (int i = 0; i < numAppClerks; i++) {
		if (appClerk[i]->getState() == CLERK_FREE && // go to a free clerk first
			appClerk[i]->getLineCount() < lineSize) { // the shortest free clerk
			myLine = i;
			lineSize = appClerk[i]->getLineCount();
			hasBribed = false;
		}
		else if (appClerk[i]->getState() == CLERK_BUSY && // or go to a busy clerk
			appClerk[i]->getLineCount() + 1 < lineSize) { // that is really 1 longer
			myLine = i;
			lineSize = appClerk[i]->getLineCount() + 1;
			hasBribed = false;
		}
		else if (appClerk[i]->getLineCount() + 2 < lineSize) { // get in a break line if its much shorter
			myLine = i;
			lineSize = appClerk[i]->getLineCount() + 2;
			hasBribed = false;
		}

		// check to see if bribing is worth it
		if (appClerk[i]->getState() != CLERK_BREAK && money >= 500 &&
		appClerk[i]->getBribeLineCount() < lineSize) {
			myLine = i;
			lineSize = appClerk[i]->getBribeLineCount();
			hasBribed = true;
		}
	}
	if (myLine == -1) { // in case every clerk was on break and had all customers in line
		for (int i = 0; i < numAppClerks; i++) { // go through the clerks
			if (appClerk[i]->getLineCount() < lineSize) { // pick the shortest line
				myLine = i;
				lineSize = appClerk[i]->getLineCount();
			}
		}
	}
	if (hasBribed) {
		money -= 500;
		//wait in line
		appClerk[myLine]->incrementBribeLine(); // increase line size
		printf("%s is bribing Application Clerk %d\n", name, myLine);
		printf("%s is waiting in Application Clerk %d's Bribe line\n", name, myLine);
		appClerk[myLine]->waitOnBribeLineCV(); // sleep until the clerk wakes me up
		appClerk[myLine]->decrementBribeLine(); // i have been called up to clerk so line goes down
	}
	else {
		//wait in line
		appClerk[myLine]->incrementLine(); // increase line size
		printf("%s is waiting in Application Clerk %d's line\n", name, myLine);
		appClerk[myLine]->waitOnLineCV(); // sleep until the clerk wakes me up
		appClerk[myLine]->decrementLine(); // i have been called up to clerk so line goes down
	}

	// now I am with the clerk
	printf("%s is with Application Clerk %d\n", name, myLine);
	appClerk[myLine]->setState(CLERK_BUSY); // ensure that the clerk is busy
	appClerk[myLine]->acquireLock(); // receive the lock so interaction with clerk is atomic
	clerkLineLock->Release(); // allow other customers to get in line
							  // give my data to clerk
	appClerk[myLine]->setToFile(socialSecurity);
	printf("%s is giving Application Clerk %d his application and social\n", name, myLine);
	appClerk[myLine]->signalOnClerkCV(); // tell clerk I have given them my social
	appClerk[myLine]->waitOnClerkCV(); // wait until clerk has "filed" my social
	printf("%s is leaving Application Clerk %d\n", name, myLine);
	appClerk[myLine]->setState(CLERK_FREE); // ensure that the clerk is now free
	appClerk[myLine]->signalOnClerkCV(); // tell clerk I am leaving
	appClerk[myLine]->releaseLock(); // end of the interaction with the clerk
}

void Customer::goToPicClerk() {
	// first acquire the lock so we know the line won't change
	clerkLineLock->Acquire();
	int myLine = -1; // will be the index of the clerk whose line is chosen
	int lineSize = numCustomers + 1; // the max customers that could be in any line
	bool hasBribed = false;
	for (int i = 0; i < numPicClerks; i++) {
		if (picClerk[i]->getState() == CLERK_FREE && // go to a free clerk first
			picClerk[i]->getLineCount() < lineSize) { // the shortest free clerk
			myLine = i;
			lineSize = picClerk[i]->getLineCount();
			hasBribed = false;
		}
		else if (picClerk[i]->getState() == CLERK_BUSY && // or go to a busy clerk
			picClerk[i]->getLineCount() + 1 < lineSize) { // that is really 1 longer
			myLine = i;
			lineSize = picClerk[i]->getLineCount() + 1;
			hasBribed = false;
		}
		else if (picClerk[i]->getLineCount() + 2 < lineSize) { // get in a break line if its much shorter
			myLine = i;
			lineSize = picClerk[i]->getLineCount() + 2;
			hasBribed = false;
		}

		// check to see if bribing is worth it
		if (picClerk[i]->getState() != CLERK_BREAK && money >= 500 &&
			picClerk[i]->getBribeLineCount() < lineSize) {
			myLine = i;
			lineSize = picClerk[i]->getBribeLineCount();
			hasBribed = true;
		}
	}
	if (myLine == -1) { // in case every clerk was on break and had all customers in line
		for (int i = 0; i < numPicClerks; i++) { // go through the clerks
			if (picClerk[i]->getLineCount() < lineSize) { // pick the shortest line
				myLine = i;
				lineSize = picClerk[i]->getLineCount();
			}
		}
	}
	if (hasBribed) { // wait in the bribe line
		money -= 500;
		//wait in line
		picClerk[myLine]->incrementBribeLine(); // increase line size
		printf("%s is bribing Picture Clerk %d\n", name, myLine);
		printf("%s is waiting in Picture Clerk %d's Bribe line\n", name, myLine);
		picClerk[myLine]->waitOnBribeLineCV(); // sleep until the clerk wakes me up
		picClerk[myLine]->decrementBribeLine(); // i have been called up to clerk so line goes down
	}
	else {
		//wait in line
		picClerk[myLine]->incrementLine(); // increase line size
		printf("%s is waiting in Picture Clerk %d's line\n", name, myLine);
		picClerk[myLine]->waitOnLineCV(); // sleep until the clerk wakes me up
		picClerk[myLine]->decrementLine(); // i have been called up to clerk so line goes down
	}
    // now I am with the clerk
	printf("%s is with Picture Clerk %d\n", name, myLine);
	picClerk[myLine]->setState(CLERK_BUSY); // ensure that the clerk is busy
	picClerk[myLine]->acquireLock(); // receive the lock so interaction with clerk is atomic
	clerkLineLock->Release(); // allow other customers to get in line
							  // give my data to clerk
	picClerk[myLine]->setToFile(socialSecurity);
	bool picLiked = false;
	while (!picLiked) {
		printf("%s is getting ready with Picture Clerk %d for his picture\n", name, myLine);
		picClerk[myLine]->signalOnClerkCV(); // tell clerk I am ready for my pic
		picClerk[myLine]->waitOnClerkCV(); // wait until clerk has taken my picture
		int random = rand() % 10;
		if (random == 0) {
			printf("%s did not like his picture from Picture Clerk %d\n", name, myLine);
		}
		else {
			printf("%s liked his picture from Picture Clerk %d\n", name, myLine);
			printf("%s is leaving Picture Clerk %d\n", name, myLine);
			picClerk[myLine]->setPicLiked(true); // tell the clerk I liked the picture
			picLiked = true;
		}
		picClerk[myLine]->signalOnClerkCV(); // tell clerk I am either leaving or want a new pic
		picClerk[myLine]->waitOnClerkCV(); // wait for clerk to tell me theyll retake or i can go
	}
	picClerk[myLine]->releaseLock(); // end of the interaction with the clerk
}

void Customer::goToPassClerk() {
	// first we acquire the line lock
	clerkLineLock->Acquire();
	int myLine = -1; // will be the index of the clerk whose line is chosen
	int lineSize = numCustomers + 1; // the max customers that could be in any line
	bool hasBribed = false;
	for (int i = 0; i < numPassClerks; i++) {
		if (passClerk[i]->getState() == CLERK_FREE && // go to a free clerk first
			passClerk[i]->getLineCount() < lineSize) { // the shortest free clerk
			myLine = i;
			lineSize = passClerk[i]->getLineCount();
			hasBribed = false;
		}
		else if (passClerk[i]->getState() == CLERK_BUSY && // or go to a busy clerk
			passClerk[i]->getLineCount() + 1 < lineSize) { // that is really 1 longer
			myLine = i;
			lineSize = passClerk[i]->getLineCount() + 1;
			hasBribed = false;
		}
		else if (passClerk[i]->getLineCount() + 2 < lineSize) { // get in a break line if its much shorter
			myLine = i;
			lineSize = passClerk[i]->getLineCount() + 2;
			hasBribed = false;
		}

		// check to see if bribing is worth it
		if (passClerk[i]->getState() != CLERK_BREAK && money >= 500 &&
			passClerk[i]->getBribeLineCount() < lineSize) {
			myLine = i;
			lineSize = passClerk[i]->getBribeLineCount();
			hasBribed = true;
		}
	}
	if (myLine == -1) { // in case every clerk was on break and had all customers in line
		for (int i = 0; i < numPassClerks; i++) { // go through the clerks
			if (passClerk[i]->getLineCount() < lineSize) { // pick the shortest line
				myLine = i;
				lineSize = passClerk[i]->getLineCount();
			}
		}
	}
	if (hasBribed) {
		money -= 500;
		//wait in line
		passClerk[myLine]->incrementBribeLine(); // increase line size
		printf("%s is bribing Passport Clerk %d\n", name, myLine);
		printf("%s is waiting in Passport Clerk %d's Bribe line\n", name, myLine);
		passClerk[myLine]->waitOnBribeLineCV(); // sleep until the clerk wakes me up
		passClerk[myLine]->decrementBribeLine(); // i have been called up to clerk so line goes down
	}
	else {
		//wait in line
		passClerk[myLine]->incrementLine(); // increase line size
		printf("%s is waiting in Passport Clerk %d's line\n", name, myLine);
		passClerk[myLine]->waitOnLineCV(); // sleep until the clerk wakes me up
		passClerk[myLine]->decrementLine(); // i have been called up to clerk so line goes down
	}

	// now I am with the clerk
	printf("%s is with Passport Clerk %d\n", name, myLine);
	passClerk[myLine]->setState(CLERK_BUSY); // ensure that the clerk is busy
	passClerk[myLine]->acquireLock(); // receive the lock so interaction with clerk is atomic
	clerkLineLock->Release(); // allow other customers to get in line
							  // give my data to clerk
	passClerk[myLine]->setToFile(socialSecurity);
	printf("%s is waiting for Passport Clerk %d to certify his passport\n", name, myLine);
	passClerk[myLine]->signalOnClerkCV(); // tell clerk I have given them my social
	passClerk[myLine]->waitOnClerkCV(); // wait until clerk has "filed" my social
	printf("%s is leaving Passport Clerk %d\n", name, myLine);
	passClerk[myLine]->setState(CLERK_FREE); // ensure that the clerk is now free
	passClerk[myLine]->signalOnClerkCV(); // tell clerk I am leaving
	passClerk[myLine]->releaseLock(); // end of the interaction with the clerk
}

Clerk::Clerk(int _id, char* _name) : id(_id), name(_name), state(CLERK_FREE), toFile(-1) {
	lineCount = 0;
	bribeLineCount = 0;
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
	strcat(buff, " Clerk Condition");
	clerkCV = new Condition(buff);
	buff = new char[50];
	strcat(buff, _name);
	strcat(buff, " Bribe Line Condition");
	clerkBribeLineCV = new Condition(buff);
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

void Clerk::setToFile(int num) {
	toFile = num;
}

int Clerk::getToFile() {
	return toFile;
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

PictureClerk::PictureClerk(int _id, char* _name) : Clerk(_id, _name) {
	picLiked = false;
}

void PictureClerk::setPicLiked(bool liked) { // used by the customer to tell clerk if they liked the pic
	picLiked = liked;
}

void PictureClerk::Run() {
	printf("In %s\n", getName());

	while (true) {
		clerkLineLock->Acquire();
		if (getBribeLineCount() > 0) {
			signalOnBribeLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			picLiked = false;
			while (!picLiked) {
				waitOnClerkCV(); // wait for customer to be ready to take picture
				printf("%s is taking Customer %d's picture\n", getName(), getToFile());
				for (int i = 0; i < 20; i++) { // make taking the picture take some time
					currentThread->Yield();
				}
				printf("%s has taken Customer %d's picture\n", getName(), getToFile());
				signalOnClerkCV(); // tell customer I have taken their picture
				waitOnClerkCV(); // wait on Customer to either like or dislike
				signalOnClerkCV();
			}
			customerData[getToFile()].picture = true;
			releaseLock();
			setState(CLERK_FREE);
		} else if (getLineCount()>0) { // if there is someone in line
			signalOnLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			picLiked = false;
			while (!picLiked) {
				waitOnClerkCV(); // wait for customer to be ready to take picture
				printf("%s is taking Customer %d's picture\n", getName(), getToFile());
				for (int i = 0; i < 20; i++) { // make taking the picture take some time
					currentThread->Yield();
				}
				printf("%s has taken Customer %d's picture\n", getName(), getToFile());
				signalOnClerkCV(); // tell customer I have taken their picture
				waitOnClerkCV(); // wait on Customer to either like or dislike
				if(!picLiked)
					printf("%s will retake Customer %d's picture\n", getName(), getToFile());
				signalOnClerkCV(); // tell customer I will retake the pic
			}
			customerData[getToFile()].picture = true;
			releaseLock();
			setState(CLERK_FREE);
		}
		else { // no one is in line
			setState(CLERK_BREAK); // if no one is in line clerk goes on break
			clerkLineLock->Release();
			acquireLock(); // get the clerk lock so we can wait
			printf("%s is going on break\n", getName());
			waitOnClerkCV(); // go to sleep until the manager wakes us
			printf("%s is off of break\n", getName());
			// we are now woken up
			setState(CLERK_FREE);
			releaseLock();
		}
	}
}

ApplicationClerk::ApplicationClerk(int _id, char* _name) : Clerk(_id, _name) {
}

void ApplicationClerk::Run() {
	printf("In %s\n", getName());

	while(true) {
		clerkLineLock->Acquire();
		if (getBribeLineCount() > 0) { // if there is someone in our bribe line
			signalOnBribeLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			waitOnClerkCV(); // wait for customer to give me social
							 // customer has now given social
			printf("%s is filing Customer %d's application and social\n", getName(), getToFile());
			for (int i = 0; i < 50; i++) { // make filing the social take some time
				currentThread->Yield();
			}
			customerData[getToFile()].social = true; // so the office knows i have given my social
			printf("%s has filed Customer %d's application and social\n", getName(), getToFile());
			signalOnClerkCV(); // tell customer I have filed
			waitOnClerkCV(); // wait on Customer to leave
							 // once customer has left the interaction is over
			releaseLock();
			setState(CLERK_FREE);
		} else if(getLineCount()>0) { // if there is someone in line
			signalOnLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			waitOnClerkCV(); // wait for customer to give me social
			// customer has now given social
			printf("%s is filing Customer %d's application and social\n", getName(), getToFile());
			for (int i = 0; i < 50; i++) { // make filing the social take some time
				currentThread->Yield();
			}
			customerData[getToFile()].social = true; // so the office knows i have given my social
			printf("%s has filed Customer %d's application and social\n", getName(), getToFile());
			signalOnClerkCV(); // tell customer I have filed
			waitOnClerkCV(); // wait on Customer to leave
			// once customer has left the interaction is over
			releaseLock();
			setState(CLERK_FREE);
		} else { // no one is in line
			setState(CLERK_BREAK); // if no one is in line clerk goes on break
			clerkLineLock->Release();
			acquireLock(); // get the clerk lock so we can wait
			printf("%s is going on break\n", getName());
			waitOnClerkCV(); // go to sleep until the manager wakes us
			printf("%s is off of break\n", getName());
			// we are now woken up
			setState(CLERK_FREE);
			releaseLock();
		}
	}
}

PassportClerk::PassportClerk(int _id, char* _name) : Clerk(_id, _name) {
}

void PassportClerk::Run() {
	printf("In %s\n", getName());

	while (true) {
		clerkLineLock->Acquire();
		if (getBribeLineCount() > 0) {
			signalOnBribeLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			waitOnClerkCV(); // wait for customer to be ready to take picture
			printf("%s is certifing Customer %d's passport\n", getName(), getToFile());
			for (int i = 0; i < 50; i++) { // make the certifying take some time
				currentThread->Yield();
			}
			printf("%s has certified Customer %d's passport and is giving the passport to them\n", getName(), getToFile());
			customerData[getToFile()].passport = true;
			signalOnClerkCV(); // tell customer I have given them their passport
			waitOnClerkCV(); // wait on Customer to leave
			releaseLock();
			setState(CLERK_FREE);
		}
		else if (getLineCount()>0) { // if there is someone in line
			signalOnLineCV(); // signal them to approach the clerk
			setState(CLERK_BUSY); // make the clerk busy
			acquireLock(); // aqcuire the clerk lock so interaction is atomic
			clerkLineLock->Release(); // allow other customers to find lines
			waitOnClerkCV(); // wait for customer to be ready
			printf("%s is certifing Customer %d's passport\n", getName(), getToFile());
			for (int i = 0; i < 50; i++) { // make the certifying take some time
				currentThread->Yield();
			}
			printf("%s has certified Customer %d's passport and is giving the passport to them\n", getName(), getToFile());
			customerData[getToFile()].passport = true;
			signalOnClerkCV(); // tell customer I have given them their passport
			waitOnClerkCV(); // wait on Customer to leave
			releaseLock();
			setState(CLERK_FREE);
		}
		else { // no one is in line
			setState(CLERK_BREAK); // if no one is in line clerk goes on break
			clerkLineLock->Release();
			acquireLock(); // get the clerk lock so we can wait
			printf("%s is going on break\n", getName());
			waitOnClerkCV(); // go to sleep until the manager wakes us
			printf("%s is off of break\n", getName());
			// we are now woken up
			setState(CLERK_FREE);
			releaseLock();
		}
	}
}

void ManagerCheckLines() {
	for (int i = 0; i < numAppClerks; i++) { // cycle through the Application Clerks
		if (appClerk[i]->getState() == CLERK_BREAK && appClerk[i]->getLineCount() > 3) { // if the clerk is on break and we must wake it
			appClerk[i]->acquireLock(); // get the lock
			printf("Manager is waking Application Clerk %d from break\n", i);
			appClerk[i]->signalOnClerkCV(); // wake the clerk
			appClerk[i]->releaseLock(); // release the lock
		}
	}
	for (int i = 0; i < numPicClerks; i++) { // cycle through the Picture Clerks
		if (picClerk[i]->getState() == CLERK_BREAK && picClerk[i]->getLineCount() > 3) { // if the clerk is on break and we must wake it
			picClerk[i]->acquireLock(); // get the lock
			printf("Manager is waking Picture Clerk %d from break\n", i);
			picClerk[i]->signalOnClerkCV(); // wake the clerk
			picClerk[i]->releaseLock(); // release the lock
		}
	}
	for (int i = 0; i < numPassClerks; i++) { // cycle through the Passport Clerks
		if (passClerk[i]->getState() == CLERK_BREAK && passClerk[i]->getLineCount() > 3) { // if the clerk is on break and we must wake it
			passClerk[i]->acquireLock(); // get the lock
			printf("Manager is waking Passport Clerk %d from break\n", i);
			passClerk[i]->signalOnClerkCV(); // wake the clerk
			passClerk[i]->releaseLock(); // release the lock
		}
	}
}

bool ManagerCheckClose() { // returns true if it is okay to close the office
	//check if it is okay to close the office
	bool doneFlag = true;
	bool allHereFlag = true;
	for (int i = 0; i < numCustomers; i++) {
		if (!customerData[i].arrived) { // if any customer has not arrived
			doneFlag = false; // set the flag to false
			allHereFlag = false;
			break;
		}
		if (!customerData[i].social) { // if any social hasn't been filed it is not time to close
			doneFlag = false;
			break;
		}
		if (!customerData[i].picture) { // if any photo hasn't been taken it is not time to close
			doneFlag = false;
			break;
		}
		if (!customerData[i].passport) {
			doneFlag = false;
			break;
		}
	}
	if (doneFlag) {
		printf("Manager is closing the Passport Office\n");
		return true;
	}
	else if(allHereFlag) { // if we aren't done but every customer is here lets check if we need to wake up clerks
		bool breakFlag = true;
		for (int i = 0; i < numAppClerks; i++) { // check if all our clerks are on break
			if (appClerk[i]->getState() != CLERK_BREAK) { 
			// if one clerk isn't on break let them keep going without waking others
				breakFlag = false;
				break;
			}
		}
		for (int i = 0; i < numPicClerks; i++) {
			if (picClerk[i]->getState() != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		for (int i = 0; i < numPassClerks; i++) {
			if (passClerk[i]->getState() != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		if (breakFlag) { // if every clerk is on break and we aren't done
			for (int i = 0; i < numAppClerks; i++) { // go through the clerks
				if (appClerk[i]->getLineCount() > 0) { // if they have someone in line but are on break
					appClerk[i]->acquireLock(); // get the lock
					printf("Manager is waking Application Clerk %d from break\n", i);
					appClerk[i]->signalOnClerkCV(); // wake the clerk
					appClerk[i]->releaseLock(); // release the lock
				}
			}
			for (int i = 0; i < numPicClerks; i++) { // go through the clerks
				if (picClerk[i]->getLineCount() > 0) { // if they have someone in line but are on break
					picClerk[i]->acquireLock(); // get the lock
					printf("Manager is waking Picture Clerk %d from break\n", i);
					picClerk[i]->signalOnClerkCV(); // wake the clerk
					picClerk[i]->releaseLock(); // release the lock
				}
			}
			for (int i = 0; i < numPassClerks; i++) {
				if (passClerk[i]->getLineCount() > 0) {
					passClerk[i]->acquireLock();
					printf("Manager is waking Passport Clerk %d from break\n", i);
					passClerk[i]->signalOnClerkCV();
					passClerk[i]->releaseLock();
				}
			}
		}
	}
	return false;
}

void Manager() {
	while (true) {
		for (int k = 0; k < 50; k++) {
			ManagerCheckLines();
			for (int i = 0; i < 60; i++) {
				currentThread->Yield(); // to slow down the manager thread
			}
		}
		if (ManagerCheckClose()) {
			SystemLock->Acquire();
			SystemCondition->Signal(SystemLock); // tell the system that the test is over
			SystemLock->Release();
			break;
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

void PicClerkStart(int index) {
	picClerk[index]->Run();
}

void PassClerkStart(int index) {
	passClerk[index]->Run();
}
	//Shows that a customer always gets in the shortest line
	//and that no two customers get in the same shortest line
void t1_shortest_line(){
	
}

void t2_manager_money(){
	
}

//Shows that every customer gets a passport
//If the total passports at the end of the simulation is equal to the number of customers, then
//the test passes
void t3_total_passport(){
	
}

//Shows that clerks go on break when there is no one in their line
void t4_clerk_break(){
	
}

//Shows that the manager wakes up workers whenever a line is too long
void t5_manager_wakeup(){
	
}

//Shows that total sales never suffer from a race condition
//Money in == money out since every customer will spend all of their money
void t6_total_sales(){
	//Whenever a customer's money is determined, add this to a global variable
	//At the end of the simulation, the total money from the customers should equal the total number that the manager has tallied up
}

//Shows that behavior is proper for senators
//Once "Senator has arrived", there should only be info printed about customers finishing at a desk
//or about the senator until the senator finished
void t7_senators(){
	
}

void Part2(){
	//Test Menu
	while(1){
		printf("Tests\n");
		for(int i = 0; i < 7; i++){
			int num = i + 1;
			printf("%d: Repeatable Test #%d\n", num, num);
		}
		printf("8: System Test\n");
		printf("9: Quit\n");
		printf("Choose a test (1-8): \n");
		int choice = -1;
		scanf("%d", &choice);
		if(choice > 9 || choice < 1){
			printf("Please choose a test between 1 and 9");
			continue;
		}
		else if (choice == 9){
			break;
		}
		else{
			SystemLock = new Lock("System Lock");
			SystemCondition = new Condition("System Condition");
			SystemLock->Acquire();
			switch(choice){
				case 1:
				printf("Running Test #1\n");
				t1_shortest_line();
				break;
				case 2:
				printf("Running Test #2\n");
				t2_manager_money();
				break;
				case 3:
				printf("Running Test #3\n");
				t3_total_passport();
				break;
				case 4:
				printf("Running Test #4\n");
				t4_clerk_break();
				break;
				case 5:
				printf("Running Test #5\n");
				t5_manager_wakeup();
				break;
				case 6:
				printf("Running Test #6\n");
				t6_total_sales();
				break;
				case 7:
				printf("Running Test #7\n");
				t7_senators();
				break;
				case 8:
				printf("Running System Test\n");
				system_test();
				break;
			}
			SystemCondition->Wait(SystemLock); // wait until whichever test you selected is done
			SystemLock->Release();
		}
	}
}
void system_test() {
	//Loop to get number of customers
	while(1){
		numCustomers = -1;
		printf("Please enter the number of customers: ");
		scanf("%d", &numCustomers);
		if(numCustomers > 50 || numCustomers < 1){
			printf("Error: Must be between 1 and 50 customers. You entered %d \n", numCustomers);
			continue;
		}
		else{
			break;
		}
	}
	
	//Loop to get number of clerks
	while(1){
		numAppClerks = -1;
		printf("Please enter the number of Application Clerks: ");
		scanf("%d", &numAppClerks);
		if(numAppClerks > 5 || numAppClerks < 1){
			printf("Error: Must be between 1 and 5 clerks. You entered %d \n", numAppClerks);
			continue;
		}
		else{
			break;
		}
	}
	
	//Loop to get number of clerks
	while(1){
		numPicClerks = -1;
		printf("Please enter the number of Picture Clerks: ");
		scanf("%d", &numPicClerks);
		if(numPicClerks > 5 || numPicClerks < 1){
			printf("Error: Must be between 1 and 5 clerks. You entered %d \n", numPicClerks);
			continue;
		}
		else{
			break;
		}
	}
	
	//Loop to get number of clerks
	while(1){
		numPassClerks = -1;
		printf("Please enter the number of Passport Clerks: ");
		scanf("%d", &numPassClerks);
		if(numPassClerks > 5 || numPassClerks < 1){
			printf("Error: Must be between 1 and 5 clerks. You entered %d \n", numPassClerks);
			continue;
		}
		else{
			break;
		}
	}
	
		while(1){
		numCashiers = -1;
		printf("Please enter the number of Cashiers: ");
		scanf("%d", &numCashiers);
		if(numCashiers > 5 || numCashiers < 1){
			printf("Error: Must be between 1 and 5 clerks. You entered %d \n", numCashiers);
			continue;
		}
		else{
			break;
		}
	}
	
	//Loop to get number of senators
	while(1){
		numSenators = -1;
		printf("Please enter the number of Senators: ");
		scanf("%d", &numSenators);
		if(numSenators > 10 || numSenators < 0){
			printf("Error: Must be between 1 and 5 senators. You entered %d \n", numSenators);
			continue;
		}
		else{
			break;
		}
	}
	printf("Number of Customers = %d\n", numCustomers);
	printf("Number of ApplicationClerks = %d\n", numAppClerks);
	printf("Number of PictureClerks = %d\n", numPicClerks);
	printf("Number of PassportClerks = %d\n", numPassClerks);
	printf("Number of Cashiers = %d\n", numCashiers);
	printf("Number of Senators = %d\n", numSenators);
	
	customerData = new CustomerData[numCustomers];
	customer = new Customer*[numCustomers];
	appClerk = new ApplicationClerk*[numAppClerks];
	picClerk = new PictureClerk*[numPicClerks];
	passClerk = new PassportClerk*[numPassClerks];
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
	for (int i = 0; i < numPicClerks; i++) {
		name = new char[10];
		sprintf(name, "Picture Clerk %d", i);
		picClerk[i] = new PictureClerk(i, name);
	}
	for (int i = 0; i < numPassClerks; i++) {
		name = new char[10];
		sprintf(name, "Passport Clerk %d", i);
		passClerk[i] = new PassportClerk(i, name);
	}

	Thread* t;
	t = new Thread("Manager");
	t->Fork((VoidFunctionPtr)Manager, 0);
	for (int i = 0; i < numAppClerks; i++) {
		t = new Thread(appClerk[i]->getName());
		t->Fork((VoidFunctionPtr)AppClerkStart, i);
	}
	for (int i = 0; i < numPicClerks; i++) {
		t = new Thread(picClerk[i]->getName());
		t->Fork((VoidFunctionPtr)PicClerkStart, i);
	}
	for (int i = 0; i < numPassClerks; i++) {
		t = new Thread(passClerk[i]->getName());
		t->Fork((VoidFunctionPtr)PassClerkStart, i);
	}
	int random;
	for (int i = 0; i < numCustomers; i++) {
		t = new Thread(customer[i]->getName());
		t->Fork((VoidFunctionPtr)CustomerStart, i);
		random = rand() % 30;
		for (int j = 0; j < random; j++) { // give time between customers
			currentThread->Yield();
		}
	}
}
