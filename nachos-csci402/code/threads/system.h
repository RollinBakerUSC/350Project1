// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"
#include "addrspace.h"

#include <vector>
#include <utility>

struct KernelLock {
	KernelLock(char* name, AddrSpace* _addrspace) {
		lock = new Lock(name);
		isToBeDeleted = false;
		numThreads = 0;
		addrspace = _addrspace;
		valid = true;
	}
	Lock* lock;
	AddrSpace* addrspace;
	bool isToBeDeleted;
	bool valid;
	int numThreads;
};

struct KernelCondition {
	KernelCondition(char* name, AddrSpace* _addrspace) {
		condition = new Condition(name);
		isToBeDeleted = false;
		numThreads = 0;
		addrspace = _addrspace;
		valid = true;
	}
	Condition* condition;
	AddrSpace* addrspace;
	bool isToBeDeleted;
	bool valid;
	int numThreads;
};


struct Process {
	Process(AddrSpace* _space) {
		space = _space;
		numThreads = 1; // initialize to 1 for the thread who made the process
		threadStacks = new std::vector<std::pair<Thread*, unsigned int>* >;
	}
	AddrSpace* space;
	int numThreads;
	std::vector<std::pair<Thread*, unsigned int>* >* threadStacks;
};

struct IPTEntry{

	TranslationEntry entry;
	AddrSpace* owner;
};

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

extern std::vector<KernelLock*>* kernelLockTable;
extern Lock* kernelLockLock;
extern std::vector<KernelCondition*>* kernelCVTable;
extern Lock* kernelCVLock;

extern BitMap* mainMemoryBitMap;
extern Lock* bitMapLock;

extern std::vector<Process*>* processTable;
extern Lock* processLock;

extern IPTEntry* IPT;
extern Lock* IPTLock;

extern Lock* outputLock;

extern int currentTLB;

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
