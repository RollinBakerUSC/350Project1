#include "syscall.h"
#include "setup.h"

void checkSenator(int id) {
	if(GetMV(senatorFlag, 0) == 1) {
		Acquire(outputLock);
		Print("Customer ", 9);
		PrintInt(id);
		Print(" is going outside the office because there is a Senator present.\n", 65);
		Release(outputLock);
		Acquire(outsideLock);
		SetMV(customerOutside, id, 1); /* mark that this customer is outside */
		Wait(senatorCV, outsideLock); /* wait until the senator is all done */
		Release(outsideLock);
	}
}

void goToAppClerk(int id) {
	int myLine, lineSize;
	int i; /* for loops */
	int clerkState, lineLength;
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
		hasBribed = false;
		for(i = 0; i < NUM_APPCLERKS; i++) {
			clerkState = GetMV(appClerkState, i);
			lineLength = GetMV(appClerkLineCount, i);
			if(clerkState == 0 &&
			lineLength < lineSize) {
				myLine = i;
				lineSize = lineLength;
				hasBribed = false;
			}
			else if(clerkState == 1 &&
			lineLength + 1 < lineSize) {
				myLine = i;
				lineSize = lineLength + 1;
				hasBribed = false;
			}
			else if(lineLength + 2 < lineSize) {
				myLine = i;
				lineSize = lineLength + 2;
				hasBribed = false;
			}

			if(clerkState != 2 &&
			GetMV(customerMoney, id) >= 50 &&
			GetMV(appClerkBLineCount, i) < lineSize) {
				myLine = i;
				lineSize = GetMV(appClerkBLineCount, i);
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i < NUM_APPCLERKS; i++) {
				if(GetMV(appClerkLineCount, i) < lineSize) {
					myLine = i;
					lineSize = GetMV(appClerkLineCount, i);
				}
			}
		}
		if(hasBribed == true) {
			SetMV(customerMoney, id, GetMV(customerMoney, id)-50);
			Acquire(moneyLock);
			SetMV(appClerkMoney, myLine, GetMV(appClerkMoney, myLine)+50);
			Release(moneyLock);
			/* wait in line now */
			SetMV(appClerkBLineCount, myLine, GetMV(appClerkBLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Application Clerk ", 48);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(appClerkMutex[myLine].bribeLineCV, lineLock);
			SetMV(appClerkBLineCount, myLine, GetMV(appClerkBLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			SetMV(appClerkLineCount, myLine, GetMV(appClerkLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Application Clerk ", 50);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(appClerkMutex[myLine].lineCV, lineLock);
			SetMV(appClerkLineCount, myLine, GetMV(appClerkLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk */
	Acquire(appClerkMutex[myLine].clerkLock);
	Release(lineLock);
	SetMV(appClerkToFile, myLine, id);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Application Clerk ", 22);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	Signal(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	Wait(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	Signal(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	Release(appClerkMutex[myLine].clerkLock);
}

void goToPicClerk(int id) {
	int myLine, lineSize, random = 1;
	int i; /* for loops */
	int clerkState, lineLength;
	bool hasBribed = false, chooseLine = true, picLiked = false;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
		hasBribed = false;
		for(i = 0; i < NUM_PICCLERKS; i++) {
			clerkState = GetMV(picClerkState, i);
			lineLength = GetMV(picClerkLineCount, i);
			if(clerkState == 0 &&
			lineLength < lineSize) {
				myLine = i;
				lineSize = lineLength;
				hasBribed = false;
			}
			else if(clerkState == 1 &&
			lineLength + 1 < lineSize) {
				myLine = i;
				lineSize = lineLength + 1;
				hasBribed = false;
			}
			else if(lineLength + 2 < lineSize) {
				myLine = i;
				lineSize = lineLength + 2;
				hasBribed = false;
			}

			if(clerkState != 2 &&
			GetMV(customerMoney, id) >= 50 &&
			GetMV(picClerkBLineCount, i) < lineSize) {
				myLine = i;
				lineSize = GetMV(picClerkBLineCount, i);
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_PICCLERKS; i++) {
				if(GetMV(picClerkLineCount, i) < lineSize) {
					myLine = i;
					lineSize = GetMV(picClerkLineCount, i);
				}
			}
		}
		if(hasBribed == true) {
			SetMV(customerMoney, id, GetMV(customerMoney, id)-50);
			Acquire(moneyLock);
			SetMV(picClerkMoney, myLine, GetMV(picClerkMoney, myLine)+50);
			Release(moneyLock);
			/* wait in line now */
			SetMV(picClerkBLineCount, myLine, GetMV(picClerkBLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Picture Clerk ", 44);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(picClerkMutex[myLine].bribeLineCV, lineLock);
			SetMV(picClerkBLineCount, myLine, GetMV(picClerkBLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			SetMV(picClerkLineCount, myLine, GetMV(picClerkLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Picture Clerk ", 46);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(picClerkMutex[myLine].lineCV, lineLock);
			SetMV(picClerkLineCount, myLine, GetMV(picClerkLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk */
	Acquire(picClerkMutex[myLine].clerkLock);
	Release(lineLock);
	SetMV(picClerkToFile, myLine, id);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Picture Clerk ", 18);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	while(picLiked == false) {
		Signal(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		Wait(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		/* random = Rand() % 10; */
		if(random == 0) {
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" does not like their picture from Picture Clerk ", 48);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
		}
		else {
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" does like their picture from Picture Clerk ", 44);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			SetMV(picClerkPicLiked, myLine, 1);
			picLiked = true;
		}
		Signal(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		Wait(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
	}
	Release(picClerkMutex[myLine].clerkLock);
}

void goToPassClerk(int id) {
	int myLine, lineSize;
	int i; /* for loops */
	int clerkState, lineLength;
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
		hasBribed = false;
		for(i = 0; i < NUM_PASSCLERKS; i++) {
			clerkState = GetMV(passClerkState, i);
			lineLength = GetMV(passClerkLineCount, i);
			if(clerkState == 0 &&
			lineLength < lineSize) {
				myLine = i;
				lineSize = lineLength;
				hasBribed = false;
			}
			else if(clerkState == 1 &&
			lineLength + 1 < lineSize) {
				myLine = i;
				lineSize = lineLength + 1;
				hasBribed = false;
			}
			else if(lineLength + 2 < lineSize) {
				myLine = i;
				lineSize = lineLength + 2;
				hasBribed = false;
			}

			if(clerkState != 2 &&
			GetMV(customerMoney, id) >= 50 &&
			GetMV(passClerkBLineCount, i) < lineSize) {
				myLine = i;
				lineSize = GetMV(passClerkBLineCount, i);
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_PASSCLERKS; i++) {
				if(GetMV(passClerkLineCount, i) < lineSize) {
					myLine = i;
					lineSize = GetMV(passClerkLineCount, i);
				}
			}
		}
		if(hasBribed == true) {
			SetMV(customerMoney, id, GetMV(customerMoney, id)-50);
			Acquire(moneyLock);
			SetMV(passClerkMoney, myLine, GetMV(passClerkMoney, myLine)+50);
			Release(moneyLock);
			/* wait in line now */
			SetMV(passClerkBLineCount, myLine, GetMV(passClerkBLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Passport Clerk ", 45);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(passClerkMutex[myLine].bribeLineCV, lineLock);
			SetMV(passClerkBLineCount, myLine, GetMV(passClerkBLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			SetMV(passClerkLineCount, myLine, GetMV(passClerkLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Passport Clerk ", 47);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(passClerkMutex[myLine].lineCV, lineLock);
			SetMV(passClerkLineCount, myLine, GetMV(passClerkLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk */
	Acquire(passClerkMutex[myLine].clerkLock);
	Release(lineLock);
	SetMV(passClerkToFile, myLine, id);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Passport Clerk ", 19);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	Signal(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	Wait(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	Signal(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	Release(passClerkMutex[myLine].clerkLock);
}

void goToCashier(int id) {
	int myLine, lineSize;
	int i; /* for loops */
	int clerkState, lineLength;
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
		hasBribed = false;
		for(i = 0; i < NUM_CASHIERS; i++) {
			clerkState = GetMV(cashierState, i);
			lineLength = GetMV(cashierLineCount, i);
			if(clerkState == 0 &&
			lineLength < lineSize) {
				myLine = i;
				lineSize = lineLength;
				hasBribed = false;
			}
			else if(clerkState == 1 &&
			lineLength + 1 < lineSize) {
				myLine = i;
				lineSize = lineLength + 1;
				hasBribed = false;
			}
			else if(lineLength + 2 < lineSize) {
				myLine = i;
				lineSize = lineLength + 2;
				hasBribed = false;
			}

			if(clerkState != 2 &&
			GetMV(customerMoney, id) >= 50 &&
			GetMV(cashierBLineCount, i) < lineSize) {
				myLine = i;
				lineSize = GetMV(cashierBLineCount, i);
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i < NUM_CASHIERS; i++) {
				if(GetMV(cashierLineCount, i) < lineSize) {
					myLine = i;
					lineSize = GetMV(cashierLineCount, i);
				}
			}
		}
		if(hasBribed == true) {
			SetMV(customerMoney, id, GetMV(customerMoney, id)-50);
			Acquire(moneyLock);
			SetMV(cashierMoney, id, GetMV(cashierMoney, id)+50);
			Release(moneyLock);
			/* wait in line now */
			SetMV(cashierBLineCount, myLine, GetMV(cashierBLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Cashier ", 38);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(cashierMutex[myLine].bribeLineCV, lineLock);
			SetMV(cashierBLineCount, myLine, GetMV(cashierBLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			SetMV(cashierLineCount, myLine, GetMV(cashierLineCount, myLine)+1);
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Cashier ", 40);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(cashierMutex[myLine].lineCV, lineLock);
			SetMV(cashierLineCount, myLine, GetMV(cashierLineCount, myLine)-1);
			if(GetMV(senatorFlag, 0) == 1) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				SetMV(customerOutside, id, 1);
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk */
	Acquire(cashierMutex[myLine].clerkLock);
	Release(lineLock);
	SetMV(cashierToFile, myLine, id);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Cashier ", 12);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	Signal(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given $100 to Cashier ", 27);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	SetMV(customerMoney, id, GetMV(customerMoney, id)-10);
	Wait(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Signal(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Release(cashierMutex[myLine].clerkLock);
}

int main() {
	int id;
	Setup();
	Acquire(custLock);
	id = GetMV(customerIndex, 0);
	SetMV(customerIndex, 0, id+1);
	Release(custLock);

	checkSenator(id);
	SetMV(customerArrived, id, 1); /* mark that the customer has arrived */
	SetMV(customerMoney, id, 60);
	goToAppClerk(id);
	checkSenator(id);
	goToPicClerk(id);
	checkSenator(id);
	goToPassClerk(id);
	checkSenator(id);
	goToCashier(id);
	Acquire(outputLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" is leaving the Passport Office.\n", 33);
	Release(outputLock);

	Exit(0);
}
