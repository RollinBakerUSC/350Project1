#include "syscall.h"
#include "setup.h"

void senGoToApp(int id) {
	Acquire(lineLock);

	SetMV(appClerkSenInLine, 0, 1);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Application Clerk 0.\n", 53);
	Release(outputLock);
	Wait(appClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	SetMV(appClerkSenInLine, 0, 0);
	Acquire(appClerkMutex[0].clerkLock);
	Release(lineLock);
	SetMV(appClerkToFile, 0, id);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Application Clerk 0.\n", 25);
	Release(outputLock);
	Signal(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	Wait(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	Signal(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	Release(appClerkMutex[0].clerkLock);
}

void senGoToPic(int id) {
	int random;
	bool picLiked = false;
	Acquire(lineLock);

	SetMV(picClerkSenInLine, 0, 1);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Picture Clerk 0.\n", 49);
	Release(outputLock);
	Wait(picClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	SetMV(picClerkSenInLine, 0, 0);
	Acquire(picClerkMutex[0].clerkLock);
	Release(lineLock);
	SetMV(picClerkToFile, 0, id);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Picture Clerk 0.\n", 21);
	Release(outputLock);
	while(picLiked == false) {
		Signal(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		Wait(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		/* random = Rand() % 10; */
		random = 1;
		if(random == 0) {
			Acquire(outputLock);
			Print("Senator ", 8);
			PrintInt(id);
			Print(" does not like their picture from Picture Clerk 0.\n", 51);
			Release(outputLock);
		}
		else {
			Acquire(outputLock);
			Print("Senator ", 8);
			PrintInt(id);
			Print(" does like their picture from Picture Clerk 0.\n", 47);
			Release(outputLock);
			SetMV(picClerkPicLiked, 0, 1);
			picLiked = true;
		}
		Signal(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		Wait(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
	}
	Release(picClerkMutex[0].clerkLock);
}

void senGoToPass(int id){
	Acquire(lineLock);

	SetMV(passClerkSenInLine, 0, 1);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Passport Clerk 0.\n", 50);
	Release(outputLock);
	Wait(passClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	SetMV(passClerkSenInLine, 0, 0);
	Acquire(passClerkMutex[0].clerkLock);
	Release(lineLock);
	SetMV(passClerkToFile, 0, id);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Passport Clerk 0.\n", 22);
	Release(outputLock);
	Signal(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	Wait(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	Signal(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	Release(passClerkMutex[0].clerkLock);
}

void senGoToCashier(int id) {
	Acquire(lineLock);

	SetMV(cashierSenInLine, 0, 1);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Cashier 0.\n", 43);
	Release(outputLock);
	Wait(cashierMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	SetMV(cashierSenInLine, 0, 0);
	Acquire(cashierMutex[0].clerkLock);
	Release(lineLock);
	SetMV(cashierToFile, 0, id);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Cashier 0.\n", 15);
	Release(outputLock);
	Signal(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given Cashier 0 $100.\n", 27);
	Release(outputLock);
	SetMV(senatorMoney, id, GetMV(senatorMoney, id)-10);
	Wait(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Signal(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Release(cashierMutex[0].clerkLock);
}

int main() {
	int id;
	Setup();
	Acquire(senLock);
	id = GetMV(senatorIndex, 0);
	SetMV(senatorIndex, 0, id+1);
	Release(senLock);

	Acquire(senatorLock);
	SetMV(senatorFlag, 0, 1);
	Acquire(outsideLock);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is waiting to enter the Passport Office.\n", 42);
	Release(outputLock);
	/* wait until all the customers are outside */
	Wait(customerCV, outsideLock);
	SetMV(senatorArrived, id, 1);
	Release(outsideLock);
	senGoToApp(id);
	senGoToPic(id);
	senGoToPass(id);
	senGoToCashier(id);
	Acquire(outsideLock);
	/* wake up all the customers outside */
	Broadcast(senatorCV, outsideLock);
	Release(outsideLock);
	SetMV(senatorFlag, 0, 0);
	Release(senatorLock);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is leaving the Passport Office.\n", 33);
	Release(outputLock);
	Exit(0);
}