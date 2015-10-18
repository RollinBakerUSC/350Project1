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

	int clerkLineCV;
	int clerkBribeLineCV;
	int clerkCV;
	int clerkLock;

	bool senatorInLine; /* Set to true if senator is in line */
	clerkState state;
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

struct Customer customer[NUM_CUSTOMERS];
struct Customer senator[NUM_SENATORS];
struct Clerk appClerk[NUM_APPCLERKS];
struct Clerk picClerk[NUM_PICCLERKS];
struct Clerk passClerk[NUM_PASSCLERKS];
struct Clerk cashier[NUM_CASHIERS];

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
	int i; /* int to be used for loops */

	senatorFlag = false;
	lineLock = CreateLock("LineLock", 8);
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

		appClerk[i].clerkLineCV = CreateCondition("AppClerk Line", 13);
		appClerk[i].clerkBribeLineCV = CreateCondition("AppClerk BLine", 14);
		appClerk[i].clerkCV = CreateCondition("AppClerk CV", 11);
		appClerk[i].clerkLock = CreateLock("AppClerk Lock", 13);

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

		picClerk[i].clerkLineCV = CreateCondition("PicClerk Line", 13);
		picClerk[i].clerkBribeLineCV = CreateCondition("PicClerk BLine", 14);
		picClerk[i].clerkCV = CreateCondition("PicClerk CV", 11);
		picClerk[i].clerkLock = CreateLock("PicClerk Lock", 13);

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

		passClerk[i].clerkLineCV = CreateCondition("PassClerk Line", 13);
		passClerk[i].clerkBribeLineCV = CreateCondition("PassClerk BLine", 14);
		passClerk[i].clerkCV = CreateCondition("PassClerk CV", 11);
		passClerk[i].clerkLock = CreateLock("PassClerk Lock", 13);

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

		cashier[i].clerkLineCV = CreateCondition("Cashier Line", 13);
		cashier[i].clerkBribeLineCV = CreateCondition("Cashier BLine", 14);
		cashier[i].clerkCV = CreateCondition("Cashier CV", 11);
		cashier[i].clerkLock = CreateLock("Cashier Lock", 13);

		cashier[i].senatorInLine = false;
		cashier[i].state = CLERK_FREE;
	}
}