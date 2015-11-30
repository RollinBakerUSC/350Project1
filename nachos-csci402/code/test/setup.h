#ifndef SETUP_H
#define SETUP_H

#define NUM_CUSTOMERS 1
#define NUM_APPCLERKS 1
#define NUM_PICCLERKS 1
#define NUM_PASSCLERKS 1
#define NUM_CASHIERS 1
#define NUM_SENATORS 0

typedef enum {false, true} bool;

struct ClerkMutex
{
	int lineCV;
	int bribeLineCV;
	int senatorLineCV;
	int clerkCV;
	int clerkLock;
};

/* GLOBAL VARIABLES */
struct ClerkMutex appClerkMutex[NUM_APPCLERKS];
struct ClerkMutex picClerkMutex[NUM_PICCLERKS];
struct ClerkMutex passClerkMutex[NUM_PASSCLERKS];
struct ClerkMutex cashierMutex[NUM_CASHIERS];

int customerMoney, senatorMoney;
int customerArrived, senatorArrived;
int customerOutside, senatorOutside;
int customerSocial, senatorSocial;
int customerPicture, senatorPicture;
int customerPassport, senatorPassport;
int customerPaid, senatorPaid;

int appClerkMoney, picClerkMoney, passClerkMoney, cashierMoney;
int appClerkLineCount, picClerkLineCount, passClerkLineCount, cashierLineCount;
int appClerkBLineCount, picClerkBLineCount, passClerkBLineCount, cashierBLineCount;
int appClerkSenInLine, picClerkSenInLine, passClerkSenInLine, cashierSenInLine;
int appClerkToFile, picClerkToFile, passClerkToFile, cashierToFile;
/* 0 = free, 1 = busy, 2 = break */
int appClerkState, picClerkState, passClerkState, cashierState;
int picClerkPicLiked;

int custLock, senLock, appLock, picLock, passLock, cashLock;
int customerIndex, senatorIndex, appClerkIndex, picClerkIndex, passClerkIndex, cashierIndex;
int senatorFlag; /* true if senator is present, false if senator is not present */

int lineLock; /* lock to be used when customer is choosing a line */
int moneyLock; /* lock to be used when clerks take money and the manager counts money */
int senatorLock; /* lock to be used to ensure only one senator is present at a given time */
int outsideLock; /* lock to be used by customers to check if the outside door is open, meaning if a senator is present */
int outputLock; /* lock used when outputting since thread can be switched in between Print and PrintInt */

int senatorCV; /* CV for customers to wait on while a senator is present */
int customerCV; /* CV for senators to wait on while customers are still interacting with clerks */
/* END GLOBAL VARIABLES */

void Setup() {
	int i; /* int to be used for loops */

	custLock = CreateLock("CustLock", 8, 0);
	customerIndex = CreateMV("CustIndex", 9, 1, 0);
	senLock = CreateLock("SenLock", 7, 0);
	senatorIndex = CreateMV("SenIndex", 8, 1, 0);
	appLock = CreateLock("AppLock", 7, 0);
	appClerkIndex = CreateMV("AppIndex", 8, 1, 0);
	picLock = CreateLock("PicLock", 7, 0);
	picClerkIndex = CreateMV("PicIndex", 8, 1, 0);
	passLock = CreateLock("PassLock", 8, 0);
	passClerkIndex = CreateMV("PassIndex", 9, 1, 0);
	cashLock = CreateLock("CashLock", 8, 0);
	cashierIndex = CreateMV("CashIndex", 9, 1, 0);

	/* initialize global locks cvs and mvs */
	lineLock = CreateLock("LineLock", 8, 0);
	moneyLock = CreateLock("MoneyLock", 9, 0);
	senatorLock = CreateLock("SenatorLock", 11, 0);
	outsideLock = CreateLock("OutsideLock", 11, 0);
	outputLock = CreateLock("OutputLock", 10, 0);

	senatorCV = CreateCondition("SenatorCV", 9, 0);
	customerCV = CreateCondition("CustomerCV", 10, 0);

	senatorFlag = CreateMV("SenatorFlag", 11, 1, 0);

	customerMoney = CreateMV("CustomerMoney", 13, NUM_CUSTOMERS, 0);
	customerArrived = CreateMV("CustomerArrived", 15, NUM_CUSTOMERS, 0);
	customerOutside = CreateMV("CustomerOutside", 15, NUM_CUSTOMERS, 0);
	customerSocial = CreateMV("CustomerSocial", 14, NUM_CUSTOMERS, 0);
	customerPicture = CreateMV("CustomerPicture", 15, NUM_CUSTOMERS, 0);
	customerPassport = CreateMV("CustomerPassport", 16, NUM_CUSTOMERS, 0);
	customerPaid = CreateMV("CustomerPaid", 12, NUM_CUSTOMERS, 0);

	senatorMoney = CreateMV("SenatorMoney", 12, NUM_SENATORS, 0);
	senatorArrived = CreateMV("SenatorArrived", 15, NUM_SENATORS, 0);
	senatorOutside = CreateMV("SenatorOutside", 15, NUM_SENATORS, 0);
	senatorSocial = CreateMV("SenatorSocial", 14, NUM_SENATORS, 0);
	senatorPicture = CreateMV("SenatorPicture", 15, NUM_SENATORS, 0);
	senatorPassport = CreateMV("SenatorPassport", 16, NUM_SENATORS, 0);
	senatorPaid = CreateMV("SenatorPaid", 12, NUM_SENATORS, 0);

	appClerkMoney = CreateMV("AppClerkMoney", 13, NUM_APPCLERKS, 0);
	appClerkLineCount = CreateMV("AppClerkLineCount", 17, NUM_APPCLERKS, 0);
	appClerkBLineCount = CreateMV("AppClerkBLineCount", 18, NUM_APPCLERKS, 0);
	appClerkToFile = CreateMV("AppClerkToFile", 14, NUM_APPCLERKS, 0);
	appClerkSenInLine = CreateMV("AppClerkSenInLine", 17, NUM_APPCLERKS, 0);
	appClerkState = CreateMV("AppClerkState", 13, NUM_APPCLERKS, 0);
	for(i = 0; i < NUM_APPCLERKS; i++) {
		appClerkMutex[i].lineCV = CreateCondition("AppClerkLine", 12, i+1);
		appClerkMutex[i].bribeLineCV = CreateCondition("AppClerkBLine", 13, i+1);
		appClerkMutex[i].senatorLineCV = CreateCondition("AppClerkSLine", 13, i+1);
		appClerkMutex[i].clerkCV = CreateCondition("AppClerkCV", 10, i+1);
		appClerkMutex[i].clerkLock = CreateLock("AppClerkLock", 12, i+1);
	}

	picClerkMoney = CreateMV("PicClerkMoney", 13, NUM_PICCLERKS, 0);
	picClerkLineCount = CreateMV("PicClerkLineCount", 17, NUM_PICCLERKS, 0);
	picClerkBLineCount = CreateMV("PicClerkBLineCount", 18, NUM_PICCLERKS, 0);
	picClerkToFile = CreateMV("PicClerkToFile", 14, NUM_PICCLERKS, 0);
	picClerkSenInLine = CreateMV("PicClerkSenInLine", 17, NUM_PICCLERKS, 0);
	picClerkState = CreateMV("PicClerkState", 13, NUM_PICCLERKS, 0);
	picClerkPicLiked = CreateMV("PicClerkPicLiked", 16, NUM_PICCLERKS, 0);
	for(i = 0; i < NUM_PICCLERKS; i++) {
		picClerkMutex[i].lineCV = CreateCondition("PicClerkLine", 12, i+1);
		picClerkMutex[i].bribeLineCV = CreateCondition("PicClerkBLine", 13, i+1);
		picClerkMutex[i].senatorLineCV = CreateCondition("PicClerkSLine", 13, i+1);
		picClerkMutex[i].clerkCV = CreateCondition("PicClerkCV", 10, i+1);
		picClerkMutex[i].clerkLock = CreateLock("PicClerkLock", 12, i+1);
	}

	passClerkMoney = CreateMV("PassClerkMoney", 14, NUM_PASSCLERKS, 0);
	passClerkLineCount = CreateMV("PassClerkLineCount", 18, NUM_PASSCLERKS, 0);
	passClerkBLineCount = CreateMV("PassClerkBLineCount", 19, NUM_PASSCLERKS, 0);
	passClerkToFile = CreateMV("PassClerkToFile", 15, NUM_PASSCLERKS, 0);
	passClerkSenInLine = CreateMV("PassClerkSenInLine", 18, NUM_PASSCLERKS, 0);
	passClerkState = CreateMV("PassClerkState", 14, NUM_PASSCLERKS, 0);
	for(i = 0; i < NUM_PASSCLERKS; i++) {
		passClerkMutex[i].lineCV = CreateCondition("PassClerkLine", 13, i+1);
		passClerkMutex[i].bribeLineCV = CreateCondition("PassClerkBLine", 14, i+1);
		passClerkMutex[i].senatorLineCV = CreateCondition("PassClerkSLine", 14, i+1);
		passClerkMutex[i].clerkCV = CreateCondition("PassClerkCV", 11, i+1);
		passClerkMutex[i].clerkLock = CreateLock("PassClerkLock", 13, i+1);
	}

	cashierMoney = CreateMV("CashierMoney", 13, NUM_CASHIERS, 0);
	cashierLineCount = CreateMV("CashierLineCount", 17, NUM_CASHIERS, 0);
	cashierBLineCount = CreateMV("CashierBLineCount", 18, NUM_CASHIERS, 0);
	cashierToFile = CreateMV("CashierToFile", 14, NUM_CASHIERS, 0);
	cashierSenInLine = CreateMV("CashierSenInLine", 17, NUM_CASHIERS, 0);
	cashierState = CreateMV("CashierState", 13, NUM_CASHIERS, 0);
	for(i = 0; i < NUM_CASHIERS; i++) {
		cashierMutex[i].lineCV = CreateCondition("CashierLine", 11, i+1);
		cashierMutex[i].bribeLineCV = CreateCondition("CashierBLine", 12, i+1);
		cashierMutex[i].senatorLineCV = CreateCondition("CashierSLine", 12, i+1);
		cashierMutex[i].clerkCV = CreateCondition("CashierCV", 9, i+1);
		cashierMutex[i].clerkLock = CreateLock("CashierLock", 11, i+1);
	}
}

#endif /* SETUP_H */