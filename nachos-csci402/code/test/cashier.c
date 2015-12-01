#include "syscall.h"
#include "setup.h"

void cashierInteract(int id, bool sen) {
	int custID;
	int k; /* for looping */

	SetMV(cashierState, id, 1);
	Acquire(cashierMutex[id].clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = GetMV(cashierToFile, id);
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(custID);
		Print(" from Senator ", 14);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Senator ", 27);
		PrintInt(custID);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Senator ", 36);
		PrintInt(custID);
		Print(" after certification.\n", 22);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(custID);
		Print(" from Customer ", 15);
		PrintInt(custID);
		Print(".\n", 2);
		Release(outputLock);
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Customer ", 28);
		PrintInt(custID);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Customer ", 37);
		PrintInt(custID);
		Print(" after certification.\n", 22);
		Release(outputLock);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time */
	}
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Senator ", 22);
		PrintInt(custID);
		Print(" their completed passport.\n", 27);
		Release(outputLock);
		SetMV(senatorPaid, custID, 1);
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Customer ", 23);
		PrintInt(custID);
		Print(" their completed passport.\n", 27);
		Release(outputLock);
		SetMV(customerPaid, custID, 1);
	}
	Acquire(moneyLock);
	SetMV(cashierMoney, id, GetMV(cashierMoney, id) + 10);
	Release(moneyLock);
	/* alert the customer that they can leave */
	Signal(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(custID);
		Print(" has been given their completed passport.\n", 42);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(custID);
		Print(" has been given their completed passport.\n", 42);
		Release(outputLock);
	}
	/* wait for the customer to leave */
	Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
	Release(cashierMutex[id].clerkLock);
	SetMV(cashierState, id, 0);
}

void cashierRun(int id) {
	while(GetMV(doneFlag, 0) == 0) {
		Acquire(lineLock);
		if(GetMV(senatorFlag, 0) == 1) { /* if a senator is in the building */
			if(GetMV(cashierLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(cashierMutex[id].lineCV, lineLock);
			}
			if(GetMV(cashierBLineCount, id) > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(cashierMutex[id].bribeLineCV, lineLock);
			}
			if(GetMV(cashierSenInLine, id) == true) {
				Signal(cashierMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread */
				Acquire(outputLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" has signalled a Senator to come to their counter.\n", 51);
				Release(outputLock);
				cashierInteract(id, true);
			} else { /* else if a senator is here and not in this line */
				SetMV(cashierState, id, 2);
				Release(lineLock);
				Acquire(cashierMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk */
				Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is coming of off break.\n", 25);
				Release(outputLock);
				SetMV(cashierState, id, 0);
				Release(cashierMutex[id].clerkLock);
			}
		}
		else if(GetMV(cashierBLineCount, id) > 0) {
			Signal(cashierMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			cashierInteract(id, false);
		}
		else if(GetMV(cashierLineCount, id) > 0){
			Signal(cashierMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			cashierInteract(id, false);
		}
		else { /* go on break */
			SetMV(cashierState, id, 2);
			Release(lineLock);
			Acquire(cashierMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk */
			Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is coming of off break.\n", 25);
			Release(outputLock);
			SetMV(cashierState, id, 0);
			Release(cashierMutex[id].clerkLock);
		}
	}
}

int main() {
	int id;
	Setup();
	Acquire(cashLock);
	id = GetMV(cashierIndex, 0);
	SetMV(cashierIndex, 0, id+1);
	Release(cashLock);
	cashierRun(id);
	Exit(0);
}