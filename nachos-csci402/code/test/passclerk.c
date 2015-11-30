#include "syscall.h"
#include "setup.h"

void passClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping */

	SetMV(passClerkState, id, 1); /* make busy */
	Acquire(passClerkMutex[id].clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = GetMV(passClerkToFile, id);
	if(sen == true) {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(custID);
		Print(" from Senator ", 14);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(custID);
		Print(" from Customer ", 15);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time */
	}
	/* mark that the senator has filed his social */
	if(sen == true) {
		SetMV(senatorPassport, custID, 1);
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Senator ", 29);
		PrintInt(custID);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	} else {
		SetMV(customerPassport, custID, 1);
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Customer ", 30);
		PrintInt(custID);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	}
	/* alert the customer that they can leave */
	Signal(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
	if(sen == true) {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(custID);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(custID);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	}
	/* wait for the customer to leave */
	Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
	Release(passClerkMutex[id].clerkLock);
	SetMV(passClerkState, id, 0);
}

void passClerkRun(int id) {
	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building */
			if(GetMV(passClerkLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(passClerkMutex[id].lineCV, lineLock);
			}
			if(GetMV(passClerkBLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(passClerkMutex[id].bribeLineCV, lineLock);
			}
			if(GetMV(passClerkSenInLine, id) == 1) {
			Signal(passClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread */
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			Release(outputLock);
			passClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line */
				SetMV(passClerkState, id, 2);
				Release(lineLock);
				Acquire(passClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk */
				Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				SetMV(passClerkState, id, 0);
				Release(passClerkMutex[id].clerkLock);
			}
		}
		else if(GetMV(passClerkBLineCount, id) > 0) {
			Signal(passClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			passClerkInteract(id, false);
		}
		else if(GetMV(passClerkLineCount, id) > 0){
			Signal(passClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			passClerkInteract(id, false);
		}
		else { /* go on break */
			SetMV(passClerkState, id, 2);
			Release(lineLock);
			Acquire(passClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk */
			Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			SetMV(passClerkState, id, 0);
			Release(passClerkMutex[id].clerkLock);
		}
	}
}

int main() {
	int id;
	Setup();
	Acquire(passLock);
	id = GetMV(passClerkIndex, 0);
	SetMV(passClerkIndex, 0, id+1);
	Release(passLock);
	passClerkRun(id);
	Exit(0);
}