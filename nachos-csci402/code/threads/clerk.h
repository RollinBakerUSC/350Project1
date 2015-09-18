/*
	The class Clerk which will serve as the parent class
	for ApplicationClerk, PhotoClerk, and PassportClerk
*/
#ifndef CLERK_H
#define CLERK_H

#include "synch.h"

typedef enum ClerkState {CLERK_FREE, CLERK_BUSY, CLERK_BREAK};

class Clerk {
	public:
		Clerk(int _id, char* _name);
		char* getName();
		int getLineCount();
		int getBribeLineCount();
		ClerkState getState();

		void setState(ClerkState _state);

		void incrementLine();
		void decrementLine();
		void incrementBribeLine();
		void decrementBribeLine();

		void waitOnLineCV();
		void signalOnLineCV();
		void waitOnBribeLineCV();
		void signalOnBribeLineCV();
		void waitOnClerkCV();
		void signalOnClerkCV();
		void acquireLock();
		void releaseLock();

		virtual void Run() = 0;
	private:
		int id; // a unique identifier for each clerk
		char* name; // used in debugging and output
					// convention will be clerk type + id
		ClerkState state;
		int lineCount;
		int bribeLineCount;

		Condition* clerkLineCV;
		Condition* clerkBribeLineCV;
		Condition* clerkCV;
		Lock* clerkLock;
};

#endif
