#include "syscall.h"

#define NUM_CUSTOMERS 50
#define NUM_APPCLERKS 5
#define NUM_PICCLERKS 5
#define NUM_PASSCLERKS 5
#define NUM_CASHIERS 5
#define NUM_SENATORS 10

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
	int toFile; /* Social Security of Customer to be filed */

	bool senatorInLine; /* Set to true if senator is in line */
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
	bool arrived; /* true if the customer has arrived to the passport office */
	bool outside; /* true if the customer is waiting outside the passport office */
	bool social; /* true if the customer has gone through the appClerk */
	bool picture; /* true if the customer has gone through the picClerk */
	bool passport; /* true if the customer has gone through the passClerk */
	bool paid; /* true if the customer has gone through the cashier and is done */
};

void initializeOffice();
void startOffice();

void customerRun();
void checkSenator(int id);
void goToAppClerk(struct Customer cust, int id);
void goToPicClerk(struct Customer cust, int id);
void goToPassClerk(struct Customer cust, int id);
void goToCashier(struct Customer cust, int id);
void senatorRun();
void senGoToApp(int id);
void senGoToPic(int id);
void senGoToPass(int id);
void senGoToCashier(struct Customer sen, int id);
void appClerkRun();
void appClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen);
void picClerkRun();
void picClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen);
void passClerkRun();
void passClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen);
void cashierRun();
void cashierInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen);

void Manager();
void ManagerCheckLines();
void ManagerCountMoney(int* appMoney, int* picMoney, int* passMoney, int* cashMoney);
bool ManagerCheckClose();

/* GLOBAL VARIABLES */
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

bool senatorFlag; /* true if senator is present, false if senator is not present */

int lineLock; /* lock to be used when customer is choosing a line */
int moneyLock; /* lock to be used when clerks take money and the manager counts money */
int senatorLock; /* lock to be used to ensure only one senator is present at a given time */
int outsideLock; /* lock to be used by customers to check if the outside door is open, meaning if a senator is present */

int senatorCV; /* CV for customers to wait on while a senator is present */
int customerCV; /* CV for senators to wait on while customers are still interacting with clerks */

int main() {
	initializeOffice();
	startOffice();

	Exit(0);
}

void initializeOffice() {
	int i; /* int to be used for loops */

	senatorFlag = false;
	lineLock = CreateLock("lineLock", 8);
	moneyLock = CreateLock("MoneyLock", 9);
	senatorLock = CreateLock("SenatorLock", 11);
	outsideLock = CreateLock("OutsideLock", 11);

	senatorCV = CreateCondition("SenatorCV", 9);
	customerCV = CreateCondition("CustomerCV", 10);

	/* initialize customerData and then customers */
	for(i = 0; i < NUM_CUSTOMERS; i++) {
		customerData[i].arrived = false;
		customerData[i].outside = false;
		customerData[i].social = false;
		customerData[i].picture = false;
		customerData[i].passport = false;
		customerData[i].paid = false;

		customer[i].socialSecurity = i;
		customer[i].money = 600;
		/* customer[i].money = ((Rand() % 4) * 500) + 100; */
		customer[i].name = "Customer";
	}

	/* initialized senatorData and then senators */
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

	/* initialize appClerks */
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

	/* initialize picClerks */
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

	/* initialized passClerks */
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

	/* initialize cashiers */
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
	int i; /* for loops */
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
	struct Clerk clerk = appClerk[id];
	struct ClerkMutex clerkMutex = appClerkMutex[id];

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building */
			if(clerk.lineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.lineCV, lineLock);
			}
			if(clerk.bribeLineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.bribeLineCV, lineLock);
			}
			if(clerk.senatorInLine == true) {
			Signal(clerkMutex.senatorLineCV, lineLock); /* wake the sleeping senator thread */
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			appClerkInteract(id, clerk, clerkMutex, true);
			} else { /* else if a senator is here and not in this line */
				clerk.state = CLERK_BREAK;
				Release(lineLock);
				Acquire(clerkMutex.clerkLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				/* wait/go on break until the manager wakes this clerk */
				Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
				Print("Application Clerk ", 18);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				clerk.state = CLERK_FREE;
				Release(clerkMutex.clerkLock);
			}
		}
		else if(clerk.bribeLineCount > 0) {
			Signal(clerkMutex.bribeLineCV, lineLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			appClerkInteract(id, clerk, clerkMutex, false);
		}
		else if(clerk.lineCount > 0){
			Signal(clerkMutex.lineCV, lineLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			appClerkInteract(id, clerk, clerkMutex, false);
		}
		else { /* go on break */
			clerk.state = CLERK_BREAK;
			Release(lineLock);
			Acquire(clerkMutex.clerkLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			/* wait/go on break until the manager wakes this clerk */
			Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
			Print("Application Clerk ", 18);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			clerk.state = CLERK_FREE;
			Release(clerkMutex.clerkLock);
		}
	}
}

void appClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen) {
	int custID;
	int k; /* for looping */

	clerk.state = CLERK_BUSY;
	Acquire(clerkMutex.clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = clerk.toFile;
	if(sen == true) {
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Senator ", 14);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	} else {
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Customer ", 15);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time */
	}
	/* mark that the senator has filed his social */
	if(sen == true) {
		senatorData[custID].social = true;
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Senator ", 49);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	} else {
		customerData[custID].social = true;
		Print("Application Clerk ", 18);
		PrintInt(id);
		Print(" has recorded a complete application for Customer ", 50);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	}
	/* alert the customer that they can leave */
	Signal(clerkMutex.clerkCV, clerkMutex.clerkLock);
	/* wait for the customer to leave */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
	Release(clerkMutex.clerkLock);
	clerk.state = CLERK_FREE;
}

void picClerkRun() {
	int id = GetID();
	struct Clerk clerk = picClerk[id];
	struct ClerkMutex clerkMutex = picClerkMutex[id];

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building */
			if(clerk.lineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.lineCV, lineLock);
			}
			if(clerk.bribeLineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.bribeLineCV, lineLock);
			}
			if(clerk.senatorInLine == true) {
			Signal(clerkMutex.senatorLineCV, lineLock); /* wake the sleeping senator thread */
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			picClerkInteract(id, clerk, clerkMutex, true);
			} else { /* else if a senator is here and not in this line */
				clerk.state = CLERK_BREAK;
				Release(lineLock);
				Acquire(clerkMutex.clerkLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				/* wait/go on break until the manager wakes this clerk */
				Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				clerk.state = CLERK_FREE;
				Release(clerkMutex.clerkLock);
			}
		}
		else if(clerk.bribeLineCount > 0) {
			Signal(clerkMutex.bribeLineCV, lineLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			picClerkInteract(id, clerk, clerkMutex, false);
		}
		else if(clerk.lineCount > 0){
			Signal(clerkMutex.lineCV, lineLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			picClerkInteract(id, clerk, clerkMutex, false);
			picClerkInteract(id, clerk, clerkMutex, false);
		}
		else { /* go on break */
			clerk.state = CLERK_BREAK;
			Release(lineLock);
			Acquire(clerkMutex.clerkLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			/* wait/go on break until the manager wakes this clerk */
			Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			clerk.state = CLERK_FREE;
			Release(clerkMutex.clerkLock);
		}
	}
}

void picClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen) {
	int custID;
	int k; /* for looping */
	clerk.picLiked = false;

	clerk.state = CLERK_BUSY;
	Acquire(clerkMutex.clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = clerk.toFile;
	while(clerk.picLiked == false) {
		if(sen == true) {
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(clerk.toFile);
			Print(" from Senator ", 14);
			PrintInt(clerk.toFile);
			Print(".\n", 2);
		} else {
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has received SSN ", 18);
			PrintInt(clerk.toFile);
			Print(" from Customer ", 15);
			PrintInt(clerk.toFile);
			Print(".\n", 2);
		}
		for(k = 0; k < 20; k++) {
			Yield(); /* make taking the picture take some time */
		}
		if(sen == true) {
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Senator ", 32);
			PrintInt(clerk.toFile);
			Print(".\n", 2);
		} else {
			Print("Picture Clerk ", 14);
			PrintInt(id);
			Print(" has taken a picture of Customer ", 33);
			PrintInt(clerk.toFile);
			Print(".\n", 2);
		}
		Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
		if(clerk.picLiked == false) {
			if(sen == true) {
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Senator ", 28);
				PrintInt(clerk.toFile);
				Print(" does not like their picture.\n", 30);
			} else {
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(clerk.toFile);
				Print(" does not like their picture.\n", 30);
			}
		}
		else {
			if(sen == true) {
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Senator ", 28);
				PrintInt(clerk.toFile);
				Print(" does like their picture.\n", 26);
			} else {
				Print("Picture Clerk ", 14);
				PrintInt(id);
				Print(" has been told that Customer ", 29);
				PrintInt(clerk.toFile);
				Print(" does like their picture.\n", 26);
			}
		}
		Signal(clerkMutex.clerkCV, clerkMutex.clerkLock);
	}
	/* mark that the senator has filed his social */
	if(sen == true) {
		senatorData[custID].picture = true;
	} else {
		customerData[custID].picture = true;
	}
	Release(clerkMutex.clerkLock);
	clerk.state = CLERK_FREE;
}

void passClerkRun() {
	int id = GetID();
	struct Clerk clerk = passClerk[id];
	struct ClerkMutex clerkMutex = passClerkMutex[id];

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building */
			if(clerk.lineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.lineCV, lineLock);
			}
			if(clerk.bribeLineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.bribeLineCV, lineLock);
			}
			if(clerk.senatorInLine == true) {
			Signal(clerkMutex.senatorLineCV, lineLock); /* wake the sleeping senator thread */
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			passClerkInteract(id, clerk, clerkMutex, true);
			} else { /* else if a senator is here and not in this line */
				clerk.state = CLERK_BREAK;
				Release(lineLock);
				Acquire(clerkMutex.clerkLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				/* wait/go on break until the manager wakes this clerk */
				Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
				Print("Passport Clerk ", 15);
				PrintInt(id);
				Print(" is coming off of break.\n", 25);
				clerk.state = CLERK_FREE;
				Release(clerkMutex.clerkLock);
			}
		}
		else if(clerk.bribeLineCount > 0) {
			Signal(clerkMutex.bribeLineCV, lineLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			passClerkInteract(id, clerk, clerkMutex, false);
		}
		else if(clerk.lineCount > 0){
			Signal(clerkMutex.lineCV, lineLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			passClerkInteract(id, clerk, clerkMutex, false);
		}
		else { /* go on break */
			clerk.state = CLERK_BREAK;
			Release(lineLock);
			Acquire(clerkMutex.clerkLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			/* wait/go on break until the manager wakes this clerk */
			Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
			Print("Passport Clerk ", 15);
			PrintInt(id);
			Print(" is coming off of break.\n", 25);
			clerk.state = CLERK_FREE;
			Release(clerkMutex.clerkLock);
		}
	}
}

void passClerkInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen) {
	int custID;
	int k; /* for looping */

	clerk.state = CLERK_BUSY;
	Acquire(clerkMutex.clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = clerk.toFile;
	if(sen == true) {
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Senator ", 14);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	} else {
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Customer ", 15);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time */
	}
	/* mark that the senator has filed his social */
	if(sen == true) {
		senatorData[custID].passport = true;
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Senator ", 29);
		PrintInt(clerk.toFile);
		Print(" has both their application and picture completed.\n", 51);
	} else {
		customerData[custID].passport = true;
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has determined that Customer ", 30);
		PrintInt(clerk.toFile);
		Print(" has both their application and picture completed.\n", 51);
	}
	/* alert the customer that they can leave */
	Signal(clerkMutex.clerkCV, clerkMutex.clerkLock);
	if(sen == true) {
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(clerk.toFile);
		Print(" has both their application and picture completed.\n", 51);
	} else {
		Print("Passport Clerk ", 15);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(clerk.toFile);
		Print(" has both their application and picture completed.\n", 51);
	}
	/* wait for the customer to leave */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
	Release(clerkMutex.clerkLock);
	clerk.state = CLERK_FREE;
}

void cashierRun() {
	int id = GetID();
	struct Clerk clerk = cashier[id];
	struct ClerkMutex clerkMutex = cashierMutex[id];

	while(1) {
		Acquire(lineLock);
		if(senatorFlag == true) { /* if a senator is in the building */
			if(clerk.lineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.lineCV, lineLock);
			}
			if(clerk.bribeLineCount > 0) {
				/* wake up the customers so they leave the office */
				Broadcast(clerkMutex.bribeLineCV, lineLock);
			}
			if(clerk.senatorInLine == true) {
			Signal(clerkMutex.senatorLineCV, lineLock); /* wake the sleeping senator thread */
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Senator to come to their counter.\n", 51);
			cashierInteract(id, clerk, clerkMutex, true);
			} else { /* else if a senator is here and not in this line */
				clerk.state = CLERK_BREAK;
				Release(lineLock);
				Acquire(clerkMutex.clerkLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is going on break.\n", 20);
				/* wait/go on break until the manager wakes this clerk */
				Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
				Print("Cashier ", 8);
				PrintInt(id);
				Print(" is coming of off break.\n", 25);
				clerk.state = CLERK_FREE;
				Release(clerkMutex.clerkLock);
			}
		}
		else if(clerk.bribeLineCount > 0) {
			Signal(clerkMutex.bribeLineCV, lineLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			cashierInteract(id, clerk, clerkMutex, false);
		}
		else if(clerk.lineCount > 0){
			Signal(clerkMutex.lineCV, lineLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" has signalled a Customer to come to their counter.\n", 52);
			cashierInteract(id, clerk, clerkMutex, false);
		}
		else { /* go on break */
			clerk.state = CLERK_BREAK;
			Release(lineLock);
			Acquire(clerkMutex.clerkLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is going on break.\n", 20);
			/* wait/go on break until the manager wakes this clerk */
			Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
			Print("Cashier ", 8);
			PrintInt(id);
			Print(" is coming of off break.\n", 25);
			clerk.state = CLERK_FREE;
			Release(clerkMutex.clerkLock);
		}
	}
}

void cashierInteract(int id, struct Clerk clerk, struct ClerkMutex clerkMutex, bool sen) {
	int custID;
	int k; /* for looping */

	clerk.state = CLERK_BUSY;
	Acquire(clerkMutex.clerkLock); /* get lock to ensure interaction order */
	Release(lineLock); /* no need to hold this lock anymore */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock); /* wait for the customer to give me their social */
	/* customer has now given me his ssn */
	custID = clerk.toFile;
	if(sen == true) {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Senator ", 14);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Senator ", 27);
		PrintInt(clerk.toFile);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Senator ", 36);
		PrintInt(clerk.toFile);
		Print(" after certification.\n", 22);
	} else {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has received SSN ", 18);
		PrintInt(clerk.toFile);
		Print(" from Customer ", 15);
		PrintInt(clerk.toFile);
		Print(".\n", 2);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has verified that Customer ", 28);
		PrintInt(clerk.toFile);
		Print(" has been certified by a Passport Clerk.\n", 41);
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recieved the $100 from Customer ", 37);
		PrintInt(clerk.toFile);
		Print(" after certification.\n", 22);
	}
	for(k = 0; k < 20; k++) {
		Yield(); /* make filing the ssn take some time */
	}
	if(sen == true) {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Senator ", 22);
		PrintInt(clerk.toFile);
		Print(" their completed passport.\n", 27);
		senatorData[custID].paid = true;
	} else {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has provided Customer ", 23);
		PrintInt(clerk.toFile);
		Print(" their completed passport.\n", 27);
		customerData[custID].paid = true;
	}
	Acquire(moneyLock);
	clerk.money += 100;
	Release(moneyLock);
	/* alert the customer that they can leave */
	Signal(clerkMutex.clerkCV, clerkMutex.clerkLock);
	if(sen == true) {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Senator ", 22);
		PrintInt(clerk.toFile);
		Print(" has been given their completed passport.\n", 42);
		senatorData[custID].paid = true;
	} else {
		Print("Cashier ", 8);
		PrintInt(id);
		Print(" has recorded Customer ", 23);
		PrintInt(clerk.toFile);
		Print(" has been given their completed passport.\n", 42);
	}
	/* wait for the customer to leave */
	Wait(clerkMutex.clerkCV, clerkMutex.clerkLock);
	Release(clerkMutex.clerkLock);
	clerk.state = CLERK_FREE;
}

void Manager() {
	int i, k; /* for loops */
	int appMoney = 0, picMoney = 0, passMoney = 0, cashMoney = 0;
	while(1) {
		for(k = 0; i < 60; k++) {
			ManagerCheckLines();
			for(i = 0; i < 70; i++) {
				Yield(); /* slow down the manager */
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
	int i; /* for loops */
	bool customersGone = true;

	for(i = 0; i < NUM_APPCLERKS; i++) {
		if(appClerk[i].state == CLERK_BREAK &&
		(appClerk[i].lineCount > 2 || (appClerk[i].senatorInLine == true) ||
		(appClerk[i].lineCount > 0 && senatorFlag == true))) {
			Acquire(appClerkMutex[i].clerkLock);
			Print("Manager is waking Application Clerk ", 36);
			PrintInt(i);
			Print(" from break.\n", 13);
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
			Print("Manager is waking Picture Clerk ", 32);
			PrintInt(i);
			Print(" from break.\n", 13);
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
			Print("Manager is waking Passport Clerk ", 33);
			PrintInt(i);
			Print(" from break.\n", 13);
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
			Print("Manager is waking Cashier ", 26);
			PrintInt(i);
			Print(" from break.\n", 13);
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
		*appMoney += appClerk[i].money;
		appClerk[i].money = 0;
	}
	Print("Manager has counted a total of $", 32);
	PrintInt(*appMoney);
	Print(" for Application Clerks.\n", 25);
	for(i = 0; i < NUM_PICCLERKS; i++) {
		*picMoney += picClerk[i].money;
		picClerk[i].money = 0;
	}
	Print("Manager has counted a total of $", 32);
	PrintInt(*picMoney);
	Print(" for Picture Clerks.\n", 21);
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		*passMoney += passClerk[i].money;
		passClerk[i].money = 0;
	}
	Print("Manager has counted a total of $", 32);
	PrintInt(*passMoney);
	Print(" for Passport Clerks.\n", 22);
	for(i = 0; i < NUM_CASHIERS; i++) {
		*cashMoney += cashier[i].money;
		cashier[i].money = 0;
	}
	Print("Manager has counted a total of $", 32);
	PrintInt(*cashMoney);
	Print(" for Cashiers.\n", 15);
	total = *appMoney + *picMoney + *passMoney + *cashMoney;
	Print("Manager has counted a total of $", 32);
	PrintInt(total);
	Print(" for the Passport Office.\n", 26);
	Release(moneyLock);
}

bool ManagerCheckClose() {
	int i; /* for loops */
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
					Print("Manager is waking Application Clerk ", 36);
					PrintInt(i);
					Print(" from break.\n", 13);
					Signal(appClerkMutex[i].clerkCV, appClerkMutex[i].clerkLock);
					Release(appClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_PICCLERKS; i++) {
				if(picClerk[i].lineCount > 0) {
					Acquire(picClerkMutex[i].clerkLock);
					Print("Manager is waking Picture Clerk ", 32);
					PrintInt(i);
					Print(" from break.\n", 13);
					Signal(picClerkMutex[i].clerkCV, picClerkMutex[i].clerkLock);
					Release(picClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_PASSCLERKS; i++) {
				if(passClerk[i].lineCount > 0) {
					Acquire(passClerkMutex[i].clerkLock);
					Print("Manager is waking Passport Clerk ", 33);
					PrintInt(i);
					Print(" from break.\n", 13);
					Signal(passClerkMutex[i].clerkCV, passClerkMutex[i].clerkLock);
					Release(passClerkMutex[i].clerkLock);
				}
			}
			for(i = 0; i < NUM_CASHIERS; i++) {
				if(cashier[i].lineCount > 0) {
					Acquire(cashierMutex[i].clerkLock);
					Print("Manager is waking Cashier ", 26);
					PrintInt(i);
					Print(" from break.\n", 13);
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
	struct Customer cust = customer[id];

	checkSenator(id);
	customerData[id].arrived = true;
	goToAppClerk(cust, id);
	checkSenator(id);
	goToPicClerk(cust, id);
	checkSenator(id);
	goToPassClerk(cust, id);
	checkSenator(id);
	goToCashier(cust, id);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" is leaving the Passport Office.\n", 33);
	Exit(0);
}

void checkSenator(int id) {
	if(senatorFlag == true) {
		Print("Customer ", 9);
		PrintInt(id);
		Print(" is going outside the office because there is a Senator present.\n", 65);
		Acquire(outsideLock);
		customerData[id].outside = true; /* mark that this customer is outside */
		Wait(senatorCV, outsideLock); /* wait until the senator is all done */
		Release(outsideLock);
	}
}

void goToAppClerk(struct Customer cust, int id) {
	int myLine, lineSize;
	int i; /* for loops */
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
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
			cust.money >= 500 &&
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
			cust.money -= 500;
			Acquire(moneyLock);
			appClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now */
			appClerk[myLine].bribeLineCount++;
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Application Clerk ", 48);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(appClerkMutex[myLine].bribeLineCV, lineLock);
			appClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Application Clerk ", 50);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(appClerkMutex[i].lineCV, lineLock);
			appClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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

	/* now I am with the clerk */
	appClerk[myLine].state = CLERK_BUSY;
	Acquire(appClerkMutex[myLine].clerkLock);
	Release(lineLock);
	appClerk[myLine].toFile = id;
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Application Clerk ", 22);
	PrintInt(myLine);
	Print(".\n", 2);
	Signal(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	Wait(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	appClerk[myLine].state = CLERK_FREE;
	Signal(appClerkMutex[myLine].clerkCV, appClerkMutex[myLine].clerkLock);
	Release(appClerkMutex[myLine].clerkLock);
}

void goToPicClerk(struct Customer cust, int id) {
	int myLine, lineSize, random = 1;
	int i; /* for loops */
	bool hasBribed = false, chooseLine = true, picLiked = false;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
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
			cust.money >= 500 &&
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
			cust.money -= 500;
			Acquire(moneyLock);
			picClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now */
			picClerk[myLine].bribeLineCount++;
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Picture Clerk ", 44);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(picClerkMutex[myLine].bribeLineCV, lineLock);
			picClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Picture Clerk ", 46);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(picClerkMutex[i].lineCV, lineLock);
			picClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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

	/* now I am with the clerk */
	picClerk[myLine].state = CLERK_BUSY;
	Acquire(picClerkMutex[myLine].clerkLock);
	Release(lineLock);
	picClerk[myLine].toFile = id;
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Picture Clerk ", 18);
	PrintInt(myLine);
	Print(".\n", 2);
	while(picLiked == false) {
		Signal(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		Wait(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		/* random = Rand() % 10; */
		if(random == 0) {
			Print("Customer ", 9);
			PrintInt(id);
			Print(" does not like their picture from Picture Clerk ", 48);
			PrintInt(myLine);
			Print(".\n", 2);
		}
		else {
			Print("Customer ", 9);
			PrintInt(id);
			Print(" does like their picture from Picture Clerk ", 44);
			PrintInt(myLine);
			Print(".\n", 2);
			picClerk[myLine].picLiked = true;
			picLiked = true;
		}
		Signal(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
		Wait(picClerkMutex[myLine].clerkCV, picClerkMutex[myLine].clerkLock);
	}
	picClerk[myLine].state = CLERK_FREE;
	Release(picClerkMutex[myLine].clerkLock);
}

void goToPassClerk(struct Customer cust, int id) {
	int myLine, lineSize;
	int i; /* for loops */
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
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
			cust.money >= 500 &&
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
			cust.money -= 500;
			Acquire(moneyLock);
			passClerk[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now */
			passClerk[myLine].bribeLineCount++;
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Passport Clerk ", 45);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(passClerkMutex[myLine].bribeLineCV, lineLock);
			passClerk[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Passport Clerk ", 47);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(passClerkMutex[i].lineCV, lineLock);
			passClerk[myLine].lineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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

	/* now I am with the clerk */
	passClerk[myLine].state = CLERK_BUSY;
	Acquire(passClerkMutex[myLine].clerkLock);
	Release(lineLock);
	passClerk[myLine].toFile = id;
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Passport Clerk ", 19);
	PrintInt(myLine);
	Print(".\n", 2);
	Signal(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	Wait(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	passClerk[myLine].state = CLERK_FREE;
	Signal(passClerkMutex[myLine].clerkCV, passClerkMutex[myLine].clerkLock);
	Release(passClerkMutex[myLine].clerkLock);
}

void goToCashier(struct Customer cust, int id) {
	int myLine, lineSize;
	int i; /* for loops */
	bool hasBribed = false, chooseLine = true;
	while(chooseLine == true) {
		/* first acquire the line lock */
		Acquire(lineLock);
		myLine = -1;
		lineSize = NUM_CUSTOMERS+1; /* more than the max number of customers */
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
			cust.money >= 500 &&
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
			cust.money -= 500;
			Acquire(moneyLock);
			cashier[myLine].money += 500;
			Release(moneyLock);
			/* wait in line now */
			cashier[myLine].bribeLineCount++;
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in bribe line for Cashier ", 38);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(cashierMutex[myLine].bribeLineCV, lineLock);
			cashier[myLine].bribeLineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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
			Print("Customer ", 9);
			PrintInt(id);
			Print(" has gotten in regular line for Cashier ", 40);
			PrintInt(myLine);
			Print(".\n", 2);
			Wait(cashierMutex[i].lineCV, lineLock);
			cashier[myLine].lineCount--;
			if(senatorFlag == true) {
				Print("Customer ", 9);
				PrintInt(id);
				Print(" is going outside the office because there is a Senator present.\n", 65);
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

	/* now I am with the clerk */
	cashier[myLine].state = CLERK_BUSY;
	Acquire(cashierMutex[myLine].clerkLock);
	Release(lineLock);
	cashier[myLine].toFile = id;
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Cashier ", 12);
	PrintInt(myLine);
	Print(".\n", 2);
	Signal(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Print("Customer ", 9);
	PrintInt(id);
	Print(" has given $100 ", 16);
	PrintInt(id);
	Print(" to Cashier ", 12);
	PrintInt(myLine);
	Print(".\n", 2);
	customer[id].money -= 100;
	Wait(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	cashier[myLine].state = CLERK_FREE;
	Signal(cashierMutex[myLine].clerkCV, cashierMutex[myLine].clerkLock);
	Release(cashierMutex[myLine].clerkLock);
}

void senatorRun() {
	int id = GetID();
	struct Customer sen = senator[id];

	Acquire(senatorLock);
	senatorFlag = true;
	Acquire(outsideLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is waiting to enter the Passport Office.\n", 42);
	/* wait until all the customers are outside */
	Wait(customerCV, outsideLock);
	senatorData[id].arrived = true;
	Release(outsideLock);
	senGoToApp(id);
	senGoToPic(id);
	senGoToPass(id);
	senGoToCashier(sen, id);
	Acquire(outsideLock);
	/* wake up all the customers outside */
	Broadcast(senatorCV, outsideLock);
	Release(outsideLock);
	senatorFlag = false;
	Release(senatorLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" is leaving the Passport Office.\n", 33);
	Exit(0);
}

void senGoToApp(int id) {
	Acquire(lineLock);

	appClerk[0].senatorInLine = true;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Application Clerk 0.\n", 53);
	Wait(appClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	appClerk[0].senatorInLine = false;
	appClerk[0].state = CLERK_BUSY;
	Acquire(appClerkMutex[0].clerkLock);
	Release(lineLock);
	appClerk[0].toFile = id;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Application Clerk 0.\n", 25);
	Signal(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	Wait(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	appClerk[0].state = CLERK_FREE;
	Signal(appClerkMutex[0].clerkCV, appClerkMutex[0].clerkLock);
	Release(appClerkMutex[0].clerkLock);
}

void senGoToPic(int id) {
	int random;
	bool picLiked = false;
	Acquire(lineLock);

	picClerk[0].senatorInLine = true;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Picture Clerk 0.\n", 49);
	Wait(picClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	picClerk[0].senatorInLine = false;
	picClerk[0].state = CLERK_BUSY;
	Acquire(picClerkMutex[0].clerkLock);
	Release(lineLock);
	picClerk[0].toFile = id;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Picture Clerk 0.\n", 21);
	while(picLiked == false) {
		Signal(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		Wait(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		/* random = Rand() % 10; */
		random = 1;
		if(random == 0) {
			Print("Senator ", 8);
			PrintInt(id);
			Print(" does not like their picture from Picture Clerk 0.\n", 51);
		}
		else {
			Print("Senator ", 8);
			PrintInt(id);
			Print(" does like their picture from Picture Clerk 0.\n", 47);
			picClerk[0].picLiked = true;
			picLiked = true;
		}
		Signal(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
		Wait(picClerkMutex[0].clerkCV, picClerkMutex[0].clerkLock);
	}
	picClerk[0].state = CLERK_FREE;
	Release(picClerkMutex[0].clerkLock);
}

void senGoToPass(int id){
	Acquire(lineLock);

	passClerk[0].senatorInLine = true;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Passport Clerk 0.\n", 50);
	Wait(passClerkMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	passClerk[0].senatorInLine = false;
	passClerk[0].state = CLERK_BUSY;
	Acquire(passClerkMutex[0].clerkLock);
	Release(lineLock);
	passClerk[0].toFile = id;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Passport Clerk 0.\n", 22);
	Signal(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	Wait(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	passClerk[0].state = CLERK_FREE;
	Signal(passClerkMutex[0].clerkCV, passClerkMutex[0].clerkLock);
	Release(passClerkMutex[0].clerkLock);
}

void senGoToCashier(struct Customer sen, int id) {
	Acquire(lineLock);

	cashier[0].senatorInLine = true;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has gotten in regular line for Cashier 0.\n", 43);
	Wait(cashierMutex[0].senatorLineCV, lineLock);
	/* now the senator is with the clerk */
	cashier[0].senatorInLine = false;
	cashier[0].state = CLERK_BUSY;
	Acquire(cashierMutex[0].clerkLock);
	Release(lineLock);
	cashier[0].toFile = id;
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given SSN ", 15);
	PrintInt(id);
	Print(" to Cashier 0.\n", 15);
	Signal(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Print("Senator ", 8);
	PrintInt(id);
	Print(" has given Cashier 0 $100.\n", 27);
	sen.money -= 100;
	Wait(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	cashier[0].state = CLERK_FREE;
	Signal(cashierMutex[0].clerkCV, cashierMutex[0].clerkLock);
	Release(cashierMutex[0].clerkLock);
}
