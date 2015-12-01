#include "syscall.h"
#include "setup.h"

void picClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping */

	SetMV(picClerkState, id, 1); /* set busy */
	Acquire(picClerkMutex[id].clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = GetMV(picClerkToFile, id);
	while(GetMV(picClerkPicLiked, id) == 0) {
		if(sen == true) {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(custID);
			Print(" from Senator ", 14);
			PrintInt(custID);
			Print(".\n", 2);
			Release(outputLock);
		} else {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(custID);
			Print(" from Customer ", 15);
			PrintInt(custID);
			Print(".\n", 2);
			Release(outputLock);
		}
		for(k = 0; k < 20; k++) {
			Yield(); /* make taking the picture take some time */
		}
		if(sen == true) {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Senator ", 32);
			PrintInt(custID);
			Print(".\n", 2);
			Release(outputLock);
		} else {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Customer ", 33);
			PrintInt(custID);
			Print(".\n", 2);
			Release(outputLock);
		}
		Signal(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
		Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
		if(GetMV(picClerkPicLiked, id) == 0) {
			if(sen == true) {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Senator ", 28);
				PrintInt(custID);
				Print(" does not like their picture.\n", 30);
				Release(outputLock);
			} else {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(custID);
				Print(" does not like their picture.\n", 30);
				Release(outputLock);
			}
		}
		else {
			if(sen == true) {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Senator ", 28);
				PrintInt(custID);
				Print(" does like their picture.\n", 26);
				Release(outputLock);
			} else {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(custID);
				Print(" does like their picture.\n", 26);
				Release(outputLock);
			}
		}
		Signal(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
	}
	/* mark that the senator has filed his social */
	if(sen == true) {
		SetMV(senatorPicture, custID, 1);
	} else {
		SetMV(customerPicture, custID, 1);
	}
	SetMV(picClerkPicLiked, id, 0);
	Release(picClerkMutex[id].clerkLock);
	SetMV(picClerkState, id, 0);
}

void picClerkRun(int id) {
	while(GetMV(doneFlag, 0) == 0) {
		Acquire(lineLock);
		if(GetMV(senatorFlag, 0) == 1) { /* if a senator is in the building */
			if(GetMV(picClerkLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(picClerkMutex[id].lineCV, lineLock);
			}
			if(GetMV(picClerkBLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(picClerkMutex[id].bribeLineCV, lineLock);
			}
			if(GetMV(picClerkSenInLine, id) == 1) {
				Signal(picClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread */
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has signalled a Senator to come to their counter.\n", 51);
				Release(outputLock);
				picClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line */
				SetMV(picClerkState, id, 2);
				Release(lineLock);
				Acquire(picClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk */
				Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				SetMV(picClerkState, id, 0);
				Release(picClerkMutex[id].clerkLock);
			}
		}
		else if(GetMV(picClerkBLineCount, id) > 0) {
			Signal(picClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			picClerkInteract(id, false);
		}
		else if(GetMV(picClerkLineCount, id) > 0){
			Signal(picClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			picClerkInteract(id, false);
		}
		else { /* go on break */
			SetMV(picClerkState, id, 2);
			Release(lineLock);
			Acquire(picClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk */
			Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			SetMV(picClerkState, id, 0);
			Release(picClerkMutex[id].clerkLock);
		}
	}
}

int main() {
	int id;
	Setup();
	Acquire(picLock);
	id = GetMV(picClerkIndex, 0);
	SetMV(picClerkIndex, 0, id+1);
	Release(picLock);
	picClerkRun(id);
	Exit(0);
}