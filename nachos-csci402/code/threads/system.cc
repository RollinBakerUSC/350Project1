// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches

std::vector<KernelLock*>* kernelLockTable;
Lock* kernelLockLock;
std::vector<KernelCondition*>* kernelCVTable;
Lock* kernelCVLock;

Lock* bitMapLock;
BitMap* mainMemoryBitMap;

std::vector<Process*>* processTable;
Lock* processLock;

IPTEntry* IPT;
Lock* IPTLock;
int currentIPT;
bool PRand;

Lock* outputLock;

int currentTLB;
Lock* TLBLock;

OpenFile *swapFile;
Lock* swapFileLock;
BitMap* swapFileBitMap;
char* swapName;

int numMailBox;
Lock* mailBoxLock;

int numServers;
int serverID;

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
#endif

#ifdef NETWORK
PostOffice *postOffice;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

void populateIPT(){
}
//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;
    numServers = 1;
    serverID = 0;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
#endif
    int netname = 0;        // UNIX socket name
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	} else if(!strcmp(*argv, "-P")) {
        ASSERT(argc > 1);
        if(!strcmp(*(argv + 1), "RAND")) {
            PRand = true;
        }
    } else if(!strcmp(*argv, "-ns")) {
        ASSERT(argc > 1);
        numServers = atoi(*(argv + 1));
        argCount = 2;
    }
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
        serverID = netname;
	    argCount = 2;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    kernelLockTable = new std::vector<KernelLock*>;
    kernelLockLock = new Lock("Kernel Lock Lock");
    kernelCVTable = new std::vector<KernelCondition*>;
    kernelCVLock = new Lock("Kernel CV Lock");

    mainMemoryBitMap = new BitMap(NumPhysPages);
    bitMapLock = new Lock("BitMap Lock");

    processTable = new std::vector<Process*>;
    processLock = new Lock("Process Table Lock");

    IPT = new IPTEntry[NumPhysPages];
    IPTLock = new Lock("IPT Lock");
    for(int i = 0; i < NumPhysPages; i++){
        IPT[i].owner = NULL;
        IPT[i].entry.valid = false; //I think all you have to do
        IPT[i].entry.physicalPage = i;
    }
    currentIPT = 0;
    if(!PRand) {
        PRand = false;
    }

    outputLock = new Lock("Output Lock");

    currentTLB = 0;
    TLBLock = new Lock("TLB Lock");

    swapName = "swapFile";
    char* id = new char[10];
    sprintf(id, "%d", netname);
    strcat(swapName, id);
    swapFile = fileSystem->Open(swapName);
    if(swapFile == NULL) {
        fileSystem->Create(swapName, 0);
        swapFile = fileSystem->Open(swapName);
    } else {
        fileSystem->Remove(swapName);
        fileSystem->Create(swapName, 0);
        swapFile = fileSystem->Open(swapName);
    }
    swapFileBitMap = new BitMap(4096);
    swapFileLock = new Lock("Swap File Lock");

    numMailBox = 0;
    mailBoxLock = new Lock("Mail Box Lock");

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main", numMailBox);
    numMailBox++;
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;

    for(unsigned int i = 0; i < kernelLockTable->size(); i++) {
        if(kernelLockTable->at(i)->lock != NULL) {
            delete kernelLockTable->at(i)->lock;
        }
        delete kernelLockTable->at(i);
    }
    delete kernelLockTable;
    for(unsigned int i = 0; i < kernelCVTable->size(); i++) {
        if(kernelCVTable->at(i)->condition != NULL) {
            delete kernelCVTable->at(i)->condition;
        }
        delete kernelCVTable->at(i);
    }
    delete kernelCVTable;

    delete mainMemoryBitMap;
    delete bitMapLock;

    delete processTable;
    delete processLock;

    delete outputLock;

    delete swapFile;
    delete swapFileBitMap;
    delete swapFileLock;

    delete mailBoxLock;

    fileSystem->Remove(swapName);

    Exit(0);
}

