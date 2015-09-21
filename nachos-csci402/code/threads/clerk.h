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
		bool getSenatorInLine();
		void setSenatorInLine(bool b);
		ClerkState getState();

		void setState(ClerkState _state);
		void setToFile(int num);
		int getToFile();
		int getMoney();
		void addMoney(int m);

		void incrementLine();
		void decrementLine();
		void incrementBribeLine();
		void decrementBribeLine();

		void waitOnLineCV();
		void signalOnLineCV();
		void broadcastOnLineCV();
		void waitOnBribeLineCV();
		void signalOnBribeLineCV();
		void waitOnSenatorLineCV();
		void signalOnSenatorLineCV();
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
		bool senatorInLine;
		int toFile; // the id of the user who the clerk is interacting with
		int money; // the money this clerk has collected

		Condition* senatorLineCV;
		Condition* clerkLineCV;
		Condition* clerkBribeLineCV;
		Condition* clerkCV;
		Lock* clerkLock;
};

#endif
