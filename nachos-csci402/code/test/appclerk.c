#include "syscall.h"
#include "setup.h"

void appClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping */

	SetMV(appClerkState, id, 1);
	Acquire(appClerkMutex[id].clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = GetMV(appClerkToFile, id);
	if(sen == true) {
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(custID);
		Print(" from Senator ", 14);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Application Clerk ", 18);
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
		SetMV(senatorSocial, custID, 1);
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Senator ", 49);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		SetMV(customerSocial, custID, 1);
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Customer ", 50);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
	}
	/* alert the customer that they can leave */
	Signal(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
	/* wait for the customer to leave */
	Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
	Release(appClerkMutex[id].clerkLock);
	SetMV(appClerkState, id, 0);
}

void appClerkRun(int id) {
	while(1) {
		Acquire(lineLock);
		if(GetMV(senatorFlag, 0) == 1) { /* if a senator is in the building */
			if(GetMV(appClerkLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(appClerkMutex[id].lineCV, lineLock);
			}
			if(GetMV(appClerkBLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(appClerkMutex[id].bribeLineCV, lineLock);
			}
			if(GetMV(appClerkSenInLine, id) == 1) {
				Signal(appClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread */
				Acquire(outputLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" has signalled a Senator to come to their counter.\n", 51);
				Release(outputLock);
				appClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line */
				SetMV(appClerkState, id, 2);
				Release(lineLock);
				Acquire(appClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk */
				Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				SetMV(appClerkState, id, 0);
				Release(appClerkMutex[id].clerkLock);
			}
		}
		else if(GetMV(appClerkBLineCount, id) > 0) {
			Signal(appClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			appClerkInteract(id, false);
		}
		else if(GetMV(appClerkLineCount, id) > 0){
			Signal(appClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			appClerkInteract(id, false);
		}
		else { /* go on break */
			SetMV(appClerkState, id, 2);
			Release(lineLock);
			Acquire(appClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk */
			Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			SetMV(appClerkState, id, 0);
			Release(appClerkMutex[id].clerkLock);
		}
	}
}

int main() {
	int id;
	Setup();
	Acquire(appLock);
	id = GetMV(appClerkIndex, 0);
	SetMV(appClerkIndex, 0, id+1);
	Release(appLock);
	appClerkRun(id);
	Exit(0);
}