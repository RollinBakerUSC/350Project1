#include "syscall.h"
/*
#define NUM_CUSTOMERS 15
#define NUM_APPCLERKS 3
#define NUM_PICCLERKS 3
#define NUM_PASSCLERKS 3
#define NUM_CASHIERS 3
#define NUM_SENATORS 2

typedef enum {false, true} bool;
typedef enum {CLERK_FREE, CLERK_BUSY, CLERK_BREAK} clerkState;

struct Customer
{
	int socialSecurity;
	int money;
	char* name;
};

struct Clerk
{
	int id;
	int money;
	int lineCount;
	int bribeLineCount;
	int toFile; /* Social Security of Customer to be filed 

	bool senatorInLine; /* Set to true if senator is in line 
	bool picLiked;
	clerkState state;
};

struct ClerkMutex
{
	int lineCV;
	int bribeLineCV;
	int senatorLineCV;
	int clerkCV;
	int clerkLock;
};

struct CustomerData
{
	bool arrived; /* true if the customer has arrived to the passport office 
	bool outside; /* true if the customer is waiting outside the passport office 
	bool social; /* true if the customer has gone through the appClerk 
	bool picture; /* true if the customer has gone through the picClerk 
	bool passport; /* true if the customer has gone through the passClerk 
	bool paid; /* true if the customer has gone through the cashier and is done 
};

void initializeOffice();
void startOffice();

void customerRun();
void checkSenator(int id);
void goToAppClerk(int id);
void goToPicClerk(int id);
void goToPassClerk(int id);
void goToCashier(int id);
void senatorRun();
void senGoToApp(int id);
void senGoToPic(int id);
void senGoToPass(int id);
void senGoToCashier(int id);
void appClerkRun();
void appClerkInteract(int id, bool sen);
void picClerkRun();
void picClerkInteract(int id, bool sen);
void passClerkRun();
void passClerkInteract(int id, bool sen);
void cashierRun();
void cashierInteract(int id, bool sen);

void Manager();
void ManagerCheckLines();
void ManagerCountMoney(int* appMoney, int* picMoney, int* passMoney, int* cashMoney);
bool ManagerCheckClose();

/* GLOBAL VARIABLES 
struct Customer customer[NUM_CUSTOMERS];
struct Customer senator[NUM_SENATORS];
struct Clerk appClerk[NUM_APPCLERKS];
struct ClerkMutex appClerkMutex[NUM_APPCLERKS];
struct Clerk picClerk[NUM_PICCLERKS];
struct ClerkMutex picClerkMutex[NUM_PICCLERKS];
struct Clerk passClerk[NUM_PASSCLERKS];
struct ClerkMutex passClerkMutex[NUM_PASSCLERKS];
struct Clerk cashier[NUM_CASHIERS];
struct ClerkMutex cashierMutex[NUM_CASHIERS];

struct CustomerData customerData[NUM_CUSTOMERS];
struct CustomerData senatorData[NUM_SENATORS];

bool senatorFlag; /* true if senator is present, false if senator is not present 

int lineLock; /* lock to be used when customer is choosing a line 
int moneyLock; /* lock to be used when clerks take money and the manager counts money 
int senatorLock; /* lock to be used to ensure only one senator is present at a given time 
int outsideLock; /* lock to be used by customers to check if the outside door is open, meaning if a senator is present 
int outputLock; /* lock used when outputting since thread can be switched in between Print and PrintInt 

int senatorCV; /* CV for customers to wait on while a senator is present 
int customerCV; /* CV for senators to wait on while customers are still interacting with clerks 
*/
int main() {
	/*initializeOffice();
	startOffice();*/

	Exit(0);
}
/*
void initializeOffice() {
	int i; /* int to be used for loops 

	senatorFlag = false;
	lineLock = CreateLock("lineLock", 8);
	moneyLock = CreateLock("MoneyLock", 9);
	senatorLock = CreateLock("SenatorLock", 11);
	outsideLock = CreateLock("OutsideLock", 11);
	outputLock = CreateLock("OutputLock", 10);

	senatorCV = CreateCondition("SenatorCV", 9);
	customerCV = CreateCondition("CustomerCV", 10);

	/* initialize customerData and then customers 
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		customerData[i].arrived = false;
		customerData[i].outside = false;
		customerData[i].social = false;
		customerData[i].picture = false;
		customerData[i].passport = false;
		customerData[i].paid = false;

		customer[i].socialSecurity = i;
		customer[i].money = 600;
		/* customer[i].money = ((Rand() % 4) * 500) + 100; 
		customer[i].name = "Customer";
	}

	/* initialized senatorData and then senators 
	for(i = 0; i < NUM_SENATORS; i++) {
		senatorData[i].arrived = false;
		senatorData[i].outside = false;
		senatorData[i].social = false;
		senatorData[i].picture = false;
		senatorData[i].passport = false;
		senatorData[i].paid = false;

		senator[i].socialSecurity = i;
		senator[i].money = 100;
		senator[i].name = "Senator";
	}

	/* initialize appClerks 
	for(i = 0; i < NUM_APPCLERKS; i++) {
		appClerk[i].id = i;
		appClerk[i].money = 0;
		appClerk[i].lineCount = 0;
		appClerk[i].bribeLineCount = 0;
		appClerk[i].toFile = -1;

		appClerkMutex[i].lineCV = CreateCondition("AppClerk Line", 13);
		appClerkMutex[i].bribeLineCV = CreateCondition("AppClerk BLine", 14);
		appClerkMutex[i].senatorLineCV = CreateCondition("AppClerk SLine", 14);
		appClerkMutex[i].clerkCV = CreateCondition("AppClerk CV", 11);
		appClerkMutex[i].clerkLock = CreateLock("AppClerk Lock", 13);

		appClerk[i].senatorInLine = false;
		appClerk[i].state = CLERK_FREE;
	}

	/* initialize picClerks 
	for(i = 0; i < NUM_PICCLERKS; i++) {
		picClerk[i].id = i;
		picClerk[i].money = 0;
		picClerk[i].lineCount = 0;
		picClerk[i].bribeLineCount = 0;
		picClerk[i].toFile = -1;

		picClerkMutex[i].lineCV = CreateCondition("PicClerk Line", 13);
		picClerkMutex[i].bribeLineCV = CreateCondition("PicClerk BLine", 14);
		picClerkMutex[i].senatorLineCV = CreateCondition("PicClerk SLine", 14);
		picClerkMutex[i].clerkCV = CreateCondition("PicClerk CV", 11);
		picClerkMutex[i].clerkLock = CreateLock("PicClerk Lock", 13);

		picClerk[i].senatorInLine = false;
		picClerk[i].state = CLERK_FREE;
	}

	/* initialized passClerks 
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		passClerk[i].id = i;
		passClerk[i].money = 0;
		passClerk[i].lineCount = 0;
		passClerk[i].bribeLineCount = 0;
		passClerk[i].toFile = -1;

		passClerkMutex[i].lineCV = CreateCondition("PassClerk Line", 14);
		passClerkMutex[i].bribeLineCV = CreateCondition("PassClerk BLine", 15);
		passClerkMutex[i].senatorLineCV = CreateCondition("PassClerk SLine", 15);
		passClerkMutex[i].clerkCV = CreateCondition("PassClerk CV", 12);
		passClerkMutex[i].clerkLock = CreateLock("PassClerk Lock", 14);

		passClerk[i].senatorInLine = false;
		passClerk[i].state = CLERK_FREE;
	}

	/* initialize cashiers 
	for(i = 0; i < NUM_CASHIERS; i++) {
		cashier[i].id = i;
		cashier[i].money = 0;
		cashier[i].lineCount = 0;
		cashier[i].bribeLineCount = 0;
		cashier[i].toFile = -1;

		cashierMutex[i].lineCV = CreateCondition("Cashier Line", 12);
		cashierMutex[i].bribeLineCV = CreateCondition("Cashier BLine", 13);
		cashierMutex[i].senatorLineCV = CreateCondition("Cashier SLine", 13);
		cashierMutex[i].clerkCV = CreateCondition("Cashier CV", 10);
		cashierMutex[i].clerkLock = CreateLock("Cashier Lock", 12);

		cashier[i].senatorInLine = false;
		cashier[i].state = CLERK_FREE;
	}
}

void startOffice() {
	int i; /* for loops 
	Fork((void*)Manager, "Manager", 7, 0);
	for(i = 0; i < NUM_APPCLERKS; i++) {
		Fork((void*)appClerkRun, "Application Clerk", 17, i);
	}
	for(i = 0; i < NUM_PICCLERKS; i++) {
		Fork((void*)picClerkRun, "Picture Clerk", 13, i);
	}
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		Fork((void*)passClerkRun, "Passport Clerk", 14, i);
	}
	for(i = 0; i < NUM_CASHIERS; i++) {
		Fork((void*)cashierRun, "Cashier", 7, i);
	}
	for(i = 0; i < NUM_CUSTOMERS/2; i++) {
		Fork((void*)customerRun, "Customer", 8, i);
	}
	for(i = 0; i < NUM_SENATORS/2; i++) {
		Fork((void*)senatorRun, "Senator", 7, i);
	}
	for(i = NUM_CUSTOMERS/2; i < NUM_CUSTOMERS; i++) {
		Fork((void*)customerRun, "Customer", 8, i);
	}
	for(i = NUM_SENATORS/2; i < NUM_SENATORS; i++) {
		Fork((void*)senatorRun, "Senator", 7, i);
	}
}

void appClerkRun() {
	int id = GetID();

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building 
			if(appClerk[id].lineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(appClerkMutex[id].lineCV, lineLock);
			}
			if(appClerk[id].bribeLineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(appClerkMutex[id].bribeLineCV, lineLock);
			}
			if(appClerk[id].senatorInLine == true) {
			Signal(appClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread 
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			Release(outputLock);
			appClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line 
				appClerk[id].state = CLERK_BREAK;
				Release(lineLock);
				Acquire(appClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk 
				Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				appClerk[id].state = CLERK_FREE;
				Release(appClerkMutex[id].clerkLock);
			}
		}
		else if(appClerk[id].bribeLineCount > 0) {
			Signal(appClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			appClerkInteract(id, false);
		}
		else if(appClerk[id].lineCount > 0){
			Signal(appClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			appClerkInteract(id, false);
		}
		else { /* go on break 
			appClerk[id].state = CLERK_BREAK;
			Release(lineLock);
			Acquire(appClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk 
			Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			appClerk[id].state = CLERK_FREE;
			Release(appClerkMutex[id].clerkLock);
		}
	}
}

void appClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping 

	appClerk[id].state = CLERK_BUSY;
	Acquire(appClerkMutex[id].clerkLock); /* get lock to ensure interaction order 
	Release(lineLock); /* no need to hold this lock anymore 
	Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock); /* wait for the customer to give me their social 
	/* customer has now given me his ssn 
	custID = appClerk[id].toFile;
	if(sen == true) {
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(appClerk[id].toFile);
		Print(" from Senator ", 14);
		PrintInt(appClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(appClerk[id].toFile);
		Print(" from Customer ", 15);
		PrintInt(appClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time 
	}
	/* mark that the senator has filed his social 
	if(sen == true) {
		senatorData[custID].social = true;
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Senator ", 49);
		PrintInt(appClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		customerData[custID].social = true;
		Acquire(outputLock);
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Customer ", 50);
		PrintInt(appClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	}
	/* alert the customer that they can leave 
	Signal(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
	/* wait for the customer to leave 
	Wait(appClerkMutex[id].clerkCV, appClerkMutex[id].clerkLock);
	Release(appClerkMutex[id].clerkLock);
	appClerk[id].state = CLERK_FREE;
}

void picClerkRun() {
	int id = GetID();

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building 
			if(picClerk[id].lineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(picClerkMutex[id].lineCV, lineLock);
			}
			if(picClerk[id].bribeLineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(picClerkMutex[id].bribeLineCV, lineLock);
			}
			if(picClerk[id].senatorInLine == true) {
			Signal(picClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread 
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			Release(outputLock);
			picClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line 
				picClerk[id].state = CLERK_BREAK;
				Release(lineLock);
				Acquire(picClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk 
				Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				picClerk[id].state = CLERK_FREE;
				Release(picClerkMutex[id].clerkLock);
			}
		}
		else if(picClerk[id].bribeLineCount > 0) {
			Signal(picClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			picClerkInteract(id, false);
		}
		else if(picClerk[id].lineCount > 0){
			Signal(picClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			picClerkInteract(id, false);
		}
		else { /* go on break 
			picClerk[id].state = CLERK_BREAK;
			Release(lineLock);
			Acquire(picClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk 
			Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			picClerk[id].state = CLERK_FREE;
			Release(picClerkMutex[id].clerkLock);
		}
	}
}

void picClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping 
	picClerk[id].picLiked = false;

	picClerk[id].state = CLERK_BUSY;
	Acquire(picClerkMutex[id].clerkLock); /* get lock to ensure interaction order 
	Release(lineLock); /* no need to hold this lock anymore 
	Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock); /* wait for the customer to give me their social 
	/* customer has now given me his ssn 
	custID = picClerk[id].toFile;
	while(picClerk[id].picLiked == false) {
		if(sen == true) {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(picClerk[id].toFile);
			Print(" from Senator ", 14);
			PrintInt(picClerk[id].toFile);
			Print(".\n", 2);
			Release(outputLock);
		} else {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(picClerk[id].toFile);
			Print(" from Customer ", 15);
			PrintInt(picClerk[id].toFile);
			Print(".\n", 2);
			Release(outputLock);
		}
		for(k = 0; k < 20; k++) {
			Yield(); /* make taking the picture take some time 
		}
		if(sen == true) {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Senator ", 32);
			PrintInt(picClerk[id].toFile);
			Print(".\n", 2);
			Release(outputLock);
		} else {
			Acquire(outputLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Customer ", 33);
			PrintInt(picClerk[id].toFile);
			Print(".\n", 2);
			Release(outputLock);
		}
		Signal(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
		Wait(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
		if(picClerk[id].picLiked == false) {
			if(sen == true) {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Senator ", 28);
				PrintInt(picClerk[id].toFile);
				Print(" does not like their picture.\n", 30);
				Release(outputLock);
			} else {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(picClerk[id].toFile);
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
				PrintInt(picClerk[id].toFile);
				Print(" does like their picture.\n", 26);
				Release(outputLock);
			} else {
				Acquire(outputLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(picClerk[id].toFile);
				Print(" does like their picture.\n", 26);
				Release(outputLock);
			}
		}
		Signal(picClerkMutex[id].clerkCV, picClerkMutex[id].clerkLock);
	}
	/* mark that the senator has filed his social 
	if(sen == true) {
		senatorData[custID].picture = true;
	} else {
		customerData[custID].picture = true;
	}
	Release(picClerkMutex[id].clerkLock);
	picClerk[id].state = CLERK_FREE;
}

void passClerkRun() {
	int id = GetID();

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building 
			if(passClerk[id].lineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(passClerkMutex[id].lineCV, lineLock);
			}
			if(passClerk[id].bribeLineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(passClerkMutex[id].bribeLineCV, lineLock);
			}
			if(passClerk[id].senatorInLine == true) {
			Signal(passClerkMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread 
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			Release(outputLock);
			passClerkInteract(id, true);
			} else { /* else if a senator is here and not in this line 
				passClerk[id].state = CLERK_BREAK;
				Release(lineLock);
				Acquire(passClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk 
				Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				Release(outputLock);
				passClerk[id].state = CLERK_FREE;
				Release(passClerkMutex[id].clerkLock);
			}
		}
		else if(passClerk[id].bribeLineCount > 0) {
			Signal(passClerkMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			passClerkInteract(id, false);
		}
		else if(passClerk[id].lineCount > 0){
			Signal(passClerkMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			passClerkInteract(id, false);
		}
		else { /* go on break 
			passClerk[id].state = CLERK_BREAK;
			Release(lineLock);
			Acquire(passClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk 
			Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			Release(outputLock);
			passClerk[id].state = CLERK_FREE;
			Release(passClerkMutex[id].clerkLock);
		}
	}
}

void passClerkInteract(int id, bool sen) {
	int custID;
	int k; /* for looping 

	passClerk[id].state = CLERK_BUSY;
	Acquire(passClerkMutex[id].clerkLock); /* get lock to ensure interaction order 
	Release(lineLock); /* no need to hold this lock anymore 
	Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock); /* wait for the customer to give me their social 
	/* customer has now given me his ssn 
	custID = passClerk[id].toFile;
	if(sen == true) {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(passClerk[id].toFile);
		Print(" from Senator ", 14);
		PrintInt(passClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(passClerk[id].toFile);
		Print(" from Customer ", 15);
		PrintInt(passClerk[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time 
	}
	/* mark that the senator has filed his social 
	if(sen == true) {
		senatorData[custID].passport = true;
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Senator ", 29);
		PrintInt(passClerk[id].toFile);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	} else {
		customerData[custID].passport = true;
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Customer ", 30);
		PrintInt(passClerk[id].toFile);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	}
	/* alert the customer that they can leave 
	Signal(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
	if(sen == true) {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(passClerk[id].toFile);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(passClerk[id].toFile);
		Print(" has both their application and picture completed.\n", 51);
		Release(outputLock);
	}
	/* wait for the customer to leave 
	Wait(passClerkMutex[id].clerkCV, passClerkMutex[id].clerkLock);
	Release(passClerkMutex[id].clerkLock);
	passClerk[id].state = CLERK_FREE;
}

void cashierRun() {
	int id = GetID();

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building 
			if(cashier[id].lineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(cashierMutex[id].lineCV, lineLock);
			}
			if(cashier[id].bribeLineCount > 0) {
				/* wake up the customers so they leave the office 
				Broadcast(cashierMutex[id].bribeLineCV, lineLock);
			}
			if(cashier[id].senatorInLine == true) {
			Signal(cashierMutex[id].senatorLineCV, lineLock); /* wake the sleeping senator thread 
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			Release(outputLock);
			cashierInteract(id, true);
			} else { /* else if a senator is here and not in this line 
				cashier[id].state = CLERK_BREAK;
				Release(lineLock);
				Acquire(cashierMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				Release(outputLock);
				/* wait/go on break until the manager wakes this clerk 
				Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
				Acquire(outputLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is coming of off break.\n", 25);
				Release(outputLock);
				cashier[id].state = CLERK_FREE;
				Release(cashierMutex[id].clerkLock);
			}
		}
		else if(cashier[id].bribeLineCount > 0) {
			Signal(cashierMutex[id].bribeLineCV, lineLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			cashierInteract(id, false);
		}
		else if(cashier[id].lineCount > 0){
			Signal(cashierMutex[id].lineCV, lineLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			Release(outputLock);
			cashierInteract(id, false);
		}
		else { /* go on break 
			cashier[id].state = CLERK_BREAK;
			Release(lineLock);
			Acquire(cashierMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			Release(outputLock);
			/* wait/go on break until the manager wakes this clerk 
			Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
			Acquire(outputLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is coming of off break.\n", 25);
			Release(outputLock);
			cashier[id].state = CLERK_FREE;
			Release(cashierMutex[id].clerkLock);
		}
	}
}

void cashierInteract(int id, bool sen) {
	int custID;
	int k; /* for looping 

	cashier[id].state = CLERK_BUSY;
	Acquire(cashierMutex[id].clerkLock); /* get lock to ensure interaction order 
	Release(lineLock); /* no need to hold this lock anymore 
	Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock); /* wait for the customer to give me their social 
	/* customer has now given me his ssn 
	custID = cashier[id].toFile;
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(cashier[id].toFile);
		Print(" from Senator ", 14);
		PrintInt(cashier[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Senator ", 27);
		PrintInt(cashier[id].toFile);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Senator ", 36);
		PrintInt(cashier[id].toFile);
		Print(" after certification.\n", 22);
		Release(outputLock);
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(cashier[id].toFile);
		Print(" from Customer ", 15);
		PrintInt(cashier[id].toFile);
		Print(".\n", 2);
		Release(outputLock);
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Customer ", 28);
		PrintInt(cashier[id].toFile);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Customer ", 37);
		PrintInt(cashier[id].toFile);
		Print(" after certification.\n", 22);
		Release(outputLock);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time 
	}
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Senator ", 22);
		PrintInt(cashier[id].toFile);
		Print(" their completed passport.\n", 27);
		Release(outputLock);
		senatorData[custID].paid = true;
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Customer ", 23);
		PrintInt(cashier[id].toFile);
		Print(" their completed passport.\n", 27);
		Release(outputLock);
		customerData[custID].paid = true;
	}
	Acquire(moneyLock);
	cashier[id].money += 100;
	Release(moneyLock);
	/* alert the customer that they can leave 
	Signal(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
	if(sen == true) {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(cashier[id].toFile);
		Print(" has been given their completed passport.\n", 42);
		Release(outputLock);
		senatorData[custID].paid = true;
	} else {
		Acquire(outputLock);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(cashier[id].toFile);
		Print(" has been given their completed passport.\n", 42);
		Release(outputLock);
	}
	/* wait for the customer to leave 
	Wait(cashierMutex[id].clerkCV, cashierMutex[id].clerkLock);
	Release(cashierMutex[id].clerkLock);
	cashier[id].state = CLERK_FREE;
}

void Manager() {
	int i, k; /* for loops 
	int appMoney = 0, picMoney = 0, passMoney = 0, cashMoney = 0;
	while(1) {
		for(k = 0; k < 60; k++) {
			ManagerCheckLines();
			for(i = 0; i < 70; i++) {
				Yield(); /* slow down the manager 
			}
		}
		ManagerCountMoney(&appMoney, &picMoney, &passMoney, &cashMoney);
		if(ManagerCheckClose() == true) {
			break;
		}
	}
	Exit(0);
}

void ManagerCheckLines() {
	int i; /* for loops 
	bool customersGone = true;

	for(i = 0; i < NUM_APPCLERKS; i++) {
		if(appClerk[i].state == CLERK_BREAK &&
		(appClerk[i].lineCount > 2 || (appClerk[i].senatorInLine == true) ||
		(appClerk[i].lineCount > 0 && senatorFlag == true))) {
			Acquire(appClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Application Clerk ", 36);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(appClerkMutex[i].clerkCV, appClerkMutex[i].clerkLock);
			Release(appClerkMutex[i].clerkLock);
		}
		if(appClerk[i].state == CLERK_BUSY) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_PICCLERKS; i++) {
		if(picClerk[i].state == CLERK_BREAK &&
		(picClerk[i].lineCount > 2 || (picClerk[i].senatorInLine == true) ||
		(picClerk[i].lineCount > 0 && senatorFlag == true))) {
			Acquire(picClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Picture Clerk ", 32);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(picClerkMutex[i].clerkCV, picClerkMutex[i].clerkLock);
			Release(picClerkMutex[i].clerkLock);
		}
		if(picClerk[i].state == CLERK_BUSY) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		if(passClerk[i].state == CLERK_BREAK &&
		(passClerk[i].lineCount > 2 || (passClerk[i].senatorInLine == true) ||
		(passClerk[i].lineCount > 0 && senatorFlag == true))) {
			Acquire(passClerkMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Passport Clerk ", 33);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(passClerkMutex[i].clerkCV, passClerkMutex[i].clerkLock);
			Release(passClerkMutex[i].clerkLock);
		}
		if(passClerk[i].state == CLERK_BUSY) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_CASHIERS; i++) {
		if(cashier[i].state == CLERK_BREAK &&
		(cashier[i].lineCount > 2 || (cashier[i].senatorInLine == true) ||
		(cashier[i].lineCount > 0 && senatorFlag == true))) {
			Acquire(cashierMutex[i].clerkLock);
			Acquire(outputLock);
			Print("Manager is waking Cashier ", 26);
			PrintInt(i);
			Print(" from break.\n", 13);
			Release(outputLock);
			Signal(cashierMutex[i].clerkCV, cashierMutex[i].clerkLock);
			Release(cashierMutex[i].clerkLock);
		}
		if(cashier[i].state == CLERK_BUSY) {
			customersGone = false;
		}
	}
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		if(customerData[i].arrived == true && customerData[i].outside == false) {
			customersGone = false;
		}
	}
	if(customersGone == true && senatorFlag == true) {
		Acquire(outsideLock);
		/* tell the senator that all customers are outside 
		Signal(customerCV, outsideLock);
		Release(outsideLock);
	}
}

void ManagerCountMoney(int* appMoney, int* picMoney, int* passMoney, int* cashMoney) {
	int i; /* for loops 
	int total; /* total money 

	Acquire(moneyLock);
	for(i = 0; i < NUM_APPCLERKS; i++) {
		*appMoney += appClerk[i].money;
		appClerk[i].money = 0;
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*appMoney);
	Print(" for Application Clerks.\n", 25);
	Release(outputLock);
	for(i = 0; i < NUM_PICCLERKS; i++) {
		*picMoney += picClerk[i].money;
		picClerk[i].money = 0;
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*picMoney);
	Print(" for Picture Clerks.\n", 21);
	Release(outputLock);
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		*passMoney += passClerk[i].money;
		passClerk[i].money = 0;
	}
	Acquire(outputLock);
	Print("Manager has counted a total of $", 32);
	PrintInt(*passMoney);
	Print(" for Passport Clerks.\n", 22);
	Release(outputLock);
	for(i = 0; i < NUM_CASHIERS; i++) {
		*cashMoney += cashier[i].money;
		cashier[i].money = 0;
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
	int i; /* for loops 
	bool doneFlag = true, allHereFlag = true, breakFlag = true;
	for(i = 0; i < NUM_SENATORS; i++) {
		if(senatorData[i].arrived == false) {
			doneFlag = false;
			allHereFlag = false;
			break;
		}
		if(senatorData[i].social == false ||
		senatorData[i].picture == false ||
		senatorData[i].passport == false ||
		senatorData[i].paid == false) {
			doneFlag = false;
			break;
		}
	}
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		if(customerData[i].arrived == false) {
			doneFlag = false;
			allHereFlag = false;
			break;
		}
		if(customerData[i].social == false ||
		customerData[i].picture == false ||
		customerData[i].passport == false ||
		customerData[i].paid == false) {
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
			if(appClerk[i].state != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_PICCLERKS; i++) {
			if(picClerk[i].state != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_PASSCLERKS; i++) {
			if(passClerk[i].state != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		for(i = 0; i < NUM_CASHIERS; i++) {
			if(cashier[i].state != CLERK_BREAK) {
				breakFlag = false;
				break;
			}
		}
		if(breakFlag == true) {
			for(i = 0; i < NUM_APPCLERKS; i++) {
				if(appClerk[i].lineCount > 0) {
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
				if(picClerk[i].lineCount > 0) {
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
				if(passClerk[i].lineCount > 0) {
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
				if(cashier[i].lineCount > 0) {
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

void customerRun() {
	int id = GetID();

	checkSenator(id);
	customerData[id].arrived = true;
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

void checkSenator(int id) {
	if(senatorFlag == true) {
		Acquire(outputLock);
		Print("Customer ", 9);
		PrintInt(id);
		Print(" is going outside the office because there is a Senator present.\n", 65);
		Release(outputLock);
		Acquire(outsideLock);
		customerData[id].outside = true; /* mark that this customer is outside 
		Wait(senatorCV, outsideLock); /* wait until the senator is all done 
		Release(outsideLock);
	}
}

void goToAppClerk(int id) {
	int myLine, lineSize;
	int i; /* for loops 
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock 
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers 
		hasBribed = false;
		for(i = 0; i < NUM_APPCLERKS; i++) {
			if(appClerk[i].state == CLERK_FREE &&
			appClerk[i].lineCount < lineSize) {
				myLine = i;
				lineSize = appClerk[i].lineCount;
				hasBribed = false;
			}
			else if(appClerk[i].state == CLERK_BUSY &&
			appClerk[i].lineCount + 1 < lineSize) {
				myLine = i;
				lineSize = appClerk[i].lineCount + 1;
				hasBribed = false;
			}
			else if(appClerk[i].lineCount + 2 < lineSize) {
				myLine = i;
				lineSize = appClerk[i].lineCount + 2;
				hasBribed = false;
			}

			if(appClerk[i].state != CLERK_BREAK &&
			customer[id].money >= 500 &&
			appClerk[i].bribeLineCount < lineSize) {
				myLine = i;
				lineSize = appClerk[i].bribeLineCount;
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_APPCLERKS; i++) {
				if(appClerk[i].lineCount < lineSize) {
					myLine = i;
					lineSize = appClerk[i].lineCount;
				}
			}
		}
		if(hasBribed == true) {
			customer[id].money -= 500;
			Acquire(moneyLock);
			appClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now 
			appClerk[myLine].bribeLineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Application Clerk ", 48);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(appClerkMutex[myLine].bribeLineCV, lineLock);
			appClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			appClerk[myLine].lineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Application Clerk ", 50);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(appClerkMutex[myLine].lineCV, lineLock);
			appClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk 
	Acquire(appClerkMutex[myLine].clerkLock);
	Release(lineLock);
	appClerk[myLine].toFile = id;
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
	int i; /* for loops 
	bool hasBribed = false, chooseLine = true, picLiked = false;
	while(chooseLine == true) {
		/* first acquire the line lock 
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers 
		hasBribed = false;
		for(i = 0; i < NUM_PICCLERKS; i++) {
			if(picClerk[i].state == CLERK_FREE &&
			picClerk[i].lineCount < lineSize) {
				myLine = i;
				lineSize = picClerk[i].lineCount;
				hasBribed = false;
			}
			else if(picClerk[i].state == CLERK_BUSY &&
			picClerk[i].lineCount + 1 < lineSize) {
				myLine = i;
				lineSize = picClerk[i].lineCount + 1;
				hasBribed = false;
			}
			else if(picClerk[i].lineCount + 2 < lineSize) {
				myLine = i;
				lineSize = picClerk[i].lineCount + 2;
				hasBribed = false;
			}

			if(picClerk[i].state != CLERK_BREAK &&
			customer[id].money >= 500 &&
			picClerk[i].bribeLineCount < lineSize) {
				myLine = i;
				lineSize = picClerk[i].bribeLineCount;
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_PICCLERKS; i++) {
				if(picClerk[i].lineCount < lineSize) {
					myLine = i;
					lineSize = picClerk[i].lineCount;
				}
			}
		}
		if(hasBribed == true) {
			customer[id].money -= 500;
			Acquire(moneyLock);
			picClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now 
			picClerk[myLine].bribeLineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Picture Clerk ", 44);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(picClerkMutex[myLine].bribeLineCV, lineLock);
			picClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			picClerk[myLine].lineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Picture Clerk ", 46);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(picClerkMutex[myLine].lineCV, lineLock);
			picClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk 
	Acquire(picClerkMutex[myLine].clerkLock);
	Release(lineLock);
	picClerk[myLine].toFile = id;
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
		/* random = Rand() % 10; 
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
			picClerk[myLine].picLiked = true;
			picLiked = true;
		}
		Signal(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		Wait(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
	}
	Release(picClerkMutex[myLine].clerkLock);
}

void goToPassClerk(int id) {
	int myLine, lineSize;
	int i; /* for loops 
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock 
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers 
		hasBribed = false;
		for(i = 0; i < NUM_PASSCLERKS; i++) {
			if(passClerk[i].state == CLERK_FREE &&
			passClerk[i].lineCount < lineSize) {
				myLine = i;
				lineSize = passClerk[i].lineCount;
				hasBribed = false;
			}
			else if(passClerk[i].state == CLERK_BUSY &&
			passClerk[i].lineCount + 1 < lineSize) {
				myLine = i;
				lineSize = passClerk[i].lineCount + 1;
				hasBribed = false;
			}
			else if(passClerk[i].lineCount + 2 < lineSize) {
				myLine = i;
				lineSize = passClerk[i].lineCount + 2;
				hasBribed = false;
			}

			if(passClerk[i].state != CLERK_BREAK &&
			customer[id].money >= 500 &&
			passClerk[i].bribeLineCount < lineSize) {
				myLine = i;
				lineSize = passClerk[i].bribeLineCount;
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_PASSCLERKS; i++) {
				if(passClerk[i].lineCount < lineSize) {
					myLine = i;
					lineSize = passClerk[i].lineCount;
				}
			}
		}
		if(hasBribed == true) {
			customer[id].money -= 500;
			Acquire(moneyLock);
			passClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now 
			passClerk[myLine].bribeLineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Passport Clerk ", 45);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(passClerkMutex[myLine].bribeLineCV, lineLock);
			passClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			passClerk[myLine].lineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Passport Clerk ", 47);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(passClerkMutex[myLine].lineCV, lineLock);
			passClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk 
	Acquire(passClerkMutex[myLine].clerkLock);
	Release(lineLock);
	passClerk[myLine].toFile = id;
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
	int i; /* for loops 
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock 
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers 
		hasBribed = false;
		for(i = 0; i < NUM_CASHIERS; i++) {
			if(cashier[i].state == CLERK_FREE &&
			cashier[i].lineCount < lineSize) {
				myLine = i;
				lineSize = cashier[i].lineCount;
				hasBribed = false;
			}
			else if(cashier[i].state == CLERK_BUSY &&
			cashier[i].lineCount + 1 < lineSize) {
				myLine = i;
				lineSize = cashier[i].lineCount + 1;
				hasBribed = false;
			}
			else if(cashier[i].lineCount + 2 < lineSize) {
				myLine = i;
				lineSize = cashier[i].lineCount + 2;
				hasBribed = false;
			}

			if(cashier[i].state != CLERK_BREAK &&
			customer[id].money >= 500 &&
			cashier[i].bribeLineCount < lineSize) {
				myLine = i;
				lineSize = cashier[i].bribeLineCount;
				hasBribed = true;
			}
		}
		if(myLine == -1) {
			for(i = 0; i <NUM_CASHIERS; i++) {
				if(cashier[i].lineCount < lineSize) {
					myLine = i;
					lineSize = cashier[i].lineCount;
				}
			}
		}
		if(hasBribed == true) {
			customer[id].money -= 500;
			Acquire(moneyLock);
			cashier[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now 
			cashier[myLine].bribeLineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Cashier ", 38);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(cashierMutex[myLine].bribeLineCV, lineLock);
			cashier[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
		else {
			cashier[myLine].lineCount++;
			Acquire(outputLock);
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Cashier ", 40);
			PrintInt(myLine);
			Print(".\n", 2);
			Release(outputLock);
			Wait(cashierMutex[myLine].lineCV, lineLock);
			cashier[myLine].lineCount--;
			if(senatorFlag == true) {
				Acquire(outputLock);
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
				Release(outputLock);
				Release(lineLock);
				Acquire(outsideLock);
				customerData[id].outside = true;
				Wait(senatorCV, outsideLock);
				Release(outsideLock);
			}
			else {
				chooseLine = false;
			}
		}
	}

	/* now I am with the clerk 
	Acquire(cashierMutex[myLine].clerkLock);
	Release(lineLock);
	cashier[myLine].toFile = id;
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
	Print(" has given $100 ", 16);
	PrintInt(id);
	Print(" to Cashier ", 12);
	PrintInt(myLine);
	Print(".\n", 2);
	Release(outputLock);
	customer[id].money -= 100;
	Wait(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Signal(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Release(cashierMutex[myLine].clerkLock);
}

void senatorRun() {
	int id = GetID();
	struct Customer sen = senator[id];

	Acquire(senatorLock);
	senatorFlag = true;
	Acquire(outsideLock);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is waiting to enter the Passport Office.\n", 42);
	Release(outputLock);
	/* wait until all the customers are outside 
	Wait(customerCV, outsideLock);
	senatorData[id].arrived = true;
	Release(outsideLock);
	senGoToApp(id);
	senGoToPic(id);
	senGoToPass(id);
	senGoToCashier(id);
	Acquire(outsideLock);
	/* wake up all the customers outside 
	Broadcast(senatorCV, outsideLock);
	Release(outsideLock);
	senatorFlag = false;
	Release(senatorLock);
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is leaving the Passport Office.\n", 33);
	Release(outputLock);
	Exit(0);
}

void senGoToApp(int id) {
	Acquire(lineLock);

	appClerk[0].senatorInLine = true;
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Application Clerk 0.\n", 53);
	Release(outputLock);
	Wait(appClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk 
	appClerk[0].senatorInLine = false;
	Acquire(appClerkMutex[0].clerkLock);
	Release(lineLock);
	appClerk[0].toFile = id;
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

	picClerk[0].senatorInLine = true;
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Picture Clerk 0.\n", 49);
	Release(outputLock);
	Wait(picClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk 
	picClerk[0].senatorInLine = false;
	Acquire(picClerkMutex[0].clerkLock);
	Release(lineLock);
	picClerk[0].toFile = id;
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
		/* random = Rand() % 10; 
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
			picClerk[0].picLiked = true;
			picLiked = true;
		}
		Signal(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		Wait(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
	}
	Release(picClerkMutex[0].clerkLock);
}

void senGoToPass(int id){
	Acquire(lineLock);

	passClerk[0].senatorInLine = true;
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Passport Clerk 0.\n", 50);
	Release(outputLock);
	Wait(passClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk 
	passClerk[0].senatorInLine = false;
	Acquire(passClerkMutex[0].clerkLock);
	Release(lineLock);
	passClerk[0].toFile = id;
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

	cashier[0].senatorInLine = true;
	Acquire(outputLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Cashier 0.\n", 43);
	Release(outputLock);
	Wait(cashierMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk 
	cashier[0].senatorInLine = false;
	Acquire(cashierMutex[0].clerkLock);
	Release(lineLock);
	cashier[0].toFile = id;
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
	senator[id].money -= 100;
	Wait(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Signal(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Release(cashierMutex[0].clerkLock);
}
*/