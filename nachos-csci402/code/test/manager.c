#include "syscall.h"
#include "setup.h"

void ManagerCheckLines() {
	int i; /* for loops */
	bool customersGone = true;

	for(i = 0; i < NUM_APPCLERKS; i++) {
		if(GetMV(appClerkState, i) == 2 &&
		(GetMV(appClerkLineCount, i) > 2 || (GetMV(appClerkSenInLine, i) == 1) ||
		(GetMV(appClerkLineCount, i) > 0 && GetMV(senatorFlag, 0) == 1))) {
			Acquire(appClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Application Clerk ", 36);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(appClerkMutex[i].clerkCV, appClerkMutex[i].clerkLock);
			Release(appClerkMutex[i].clerkLock);
		}
		if(GetMV(appClerkState, i) == 1) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_PICCLERKS; i++) {
		if(GetMV(picClerkState, i) == 2 &&
		(GetMV(picClerkLineCount, i) > 2 || (GetMV(picClerkSenInLine, i) == 1) ||
		(GetMV(picClerkLineCount, i) > 0 && GetMV(senatorFlag, 0) == 1))) {
			Acquire(picClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Picture Clerk ", 32);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(picClerkMutex[i].clerkCV, picClerkMutex[i].clerkLock);
			Release(picClerkMutex[i].clerkLock);
		}
		if(GetMV(picClerkState, i) == 1) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		if(GetMV(passClerkState, i) == 2 &&
		(GetMV(passClerkLineCount, i) > 2 || (GetMV(passClerkSenInLine, i) == 1) ||
		(GetMV(passClerkLineCount, i) > 0 && GetMV(senatorFlag, 0) == 1))) {
			Acquire(passClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Passport Clerk ", 33);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(passClerkMutex[i].clerkCV, passClerkMutex[i].clerkLock);
			Release(passClerkMutex[i].clerkLock);
		}
		if(GetMV(passClerkState, i) == 1) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_CASHIERS; i++) {
		if(GetMV(cashierState, i) == 2 &&
		(GetMV(cashierLineCount, i) > 2 || (GetMV(cashierSenInLine, i) == 1) ||
		(GetMV(cashierLineCount, i) > 0 && GetMV(senatorFlag, 0) == 1))) {
			Acquire(cashierMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Cashier ", 26);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(cashierMutex[i].clerkCV, cashierMutex[i].clerkLock);
			Release(cashierMutex[i].clerkLock);
		}
		if(GetMV(cashierState, i) == 1) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		if(GetMV(customerArrived, i) == 1 && GetMV(customerOutside, i) == 0) {
			customersGone = false;
		}
	}
	if(customersGone == true && GetMV(senatorFlag, 0) == 1) {
		Acquire(outsideLock);
		/* tell the senator that all customers are outside */
		Signal(customerCV, outsideLock);
		Release(outsideLock);
	}
}

void ManagerCountMoney(int* appMoney, int* picMoney, int* passMoney, int* cashMoney) {
	int i; /* for loops */
	int total; /* total money */

	Acquire(moneyLock);
	for(i = 0; i < NUM_APPCLERKS; i++) {
		*appMoney += GetMV(appClerkMoney, i);
		SetMV(appClerkMoney, i, 0);
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*appMoney);
	Print(" for Application Clerks.\n", 25);
	Release(outputLock);
	for(i = 0; i < NUM_PICCLERKS; i++) {
		*picMoney += GetMV(picClerkMoney, i);
		SetMV(picClerkMoney, i, 0);
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*picMoney);
	Print(" for Picture Clerks.\n", 21);
	Release(outputLock);
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		*passMoney += GetMV(passClerkMoney, i);
		SetMV(passClerkMoney, i, 0);
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*passMoney);
	Print(" for Passport Clerks.\n", 22);
	Release(outputLock);
	for(i = 0; i < NUM_CASHIERS; i++) {
		*cashMoney += GetMV(cashierMoney, i);
		SetMV(cashierMoney, i, 0);
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*cashMoney);
	Print(" for Cashiers.\n", 15);
	Release(outputLock);
	total = *appMoney + *picMoney + *passMoney + *cashMoney;
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(total);
	Print(" for the Passport Office.\n", 26);
	Release(outputLock);
	Release(moneyLock);
}

bool ManagerCheckClose() {
	int i; /* for loops */
	bool doneFlag = true, allHereFlag = true, breakFlag = true;
	for(i = 0; i < NUM_SENATORS; i++) {
		if(GetMV(senatorArrived, i) == 0) {
			doneFlag = false;
			allHereFlag = false;
			break;
		}
		if(GetMV(senatorSocial, i) == 0 ||
		GetMV(senatorPicture, i) == 0 ||
		GetMV(senatorPassport, i) == 0 ||
		GetMV(senatorPaid, i) == 0) {
			doneFlag = false;
			break;
		}
	}
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		if(GetMV(customerArrived, i) == 0) {
			doneFlag = false;
			allHereFlag = false;
			break;
		}
		if(GetMV(customerSocial, i) == 0 ||
		GetMV(customerPicture, i) == 0 ||
		GetMV(customerPassport, i) == 0 ||
		GetMV(customerPaid, i) == 0) {
			doneFlag = false;
			break;
		}
	}
	if(doneFlag == true) {
		Print("Manager is closing the Passport Office.\n", 40);
		return true;
	}
	else if(allHereFlag == true) {
		for(i = 0; i < NUM_APPCLERKS; i++) {
			if(GetMV(appClerkState, i) != 2) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_PICCLERKS; i++) {
			if(GetMV(picClerkState, i) != 2) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_PASSCLERKS; i++) {
			if(GetMV(passClerkState, i) != 2) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_CASHIERS; i++) {
			if(GetMV(cashierState, i) != 2) {
				breakFlag = false;
				break;
			}
		}
		if(breakFlag == true) {
			for(i = 0; i < NUM_APPCLERKS; i++) {
				if(GetMV(appClerkLineCount, i) > 0) {
					Acquire(appClerkMutex[i].clerkLock);
					Acquire(outputLock);
					Print("Manager is waking Application Clerk ", 36);
					PrintInt(i);
					Print(" from break.\n", 13);
					Release(outputLock);
					Signal(appClerkMutex[i].clerkCV, appClerkMutex[i].clerkLock);
					Release(appClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_PICCLERKS; i++) {
				if(GetMV(picClerkLineCount, i) > 0) {
					Acquire(picClerkMutex[i].clerkLock);
					Acquire(outputLock);
					Print("Manager is waking Picture Clerk ", 32);
					PrintInt(i);
					Print(" from break.\n", 13);
					Release(outputLock);
					Signal(picClerkMutex[i].clerkCV, picClerkMutex[i].clerkLock);
					Release(picClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_PASSCLERKS; i++) {
				if(GetMV(passClerkLineCount, i) > 0) {
					Acquire(passClerkMutex[i].clerkLock);
					Acquire(outputLock);
					Print("Manager is waking Passport Clerk ", 33);
					PrintInt(i);
					Print(" from break.\n", 13);
					Release(outputLock);
					Signal(passClerkMutex[i].clerkCV, passClerkMutex[i].clerkLock);
					Release(passClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_CASHIERS; i++) {
				if(GetMV(cashierLineCount, i) > 0) {
					Acquire(cashierMutex[i].clerkLock);
					Acquire(outputLock);
					Print("Manager is waking Cashier ", 26);
					PrintInt(i);
					Print(" from break.\n", 13);
					Release(outputLock);
					Signal(cashierMutex[i].clerkCV, cashierMutex[i].clerkLock);
					Release(cashierMutex[i].clerkLock);
				}
			}
		} 
	}
	return false;
}

int main() {
	int i, k; /* for loops */
	int appMoney = 0, picMoney = 0, passMoney = 0, cashMoney = 0;
	Setup();
	while(1) {
		for(k = 0; k < 60; k++) {
			ManagerCheckLines();
			for(i = 0; i < 70; i++) {
				Yield();
			}
		}
		ManagerCountMoney(&appMoney, &picMoney, &passMoney, &cashMoney);
		if(ManagerCheckClose() == true) {
			break;
		}
	}
	Exit(0);
}