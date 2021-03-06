// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"
#include <queue>

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

struct PageTableEntry {
    TranslationEntry entry;
    int location; // 0 for swapfile, 1 for executable, 2 for neither
    int byteOffset;
    int swapLocation; // page number within the swap file
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *_executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
    void clearMem(); // clear out the memory upon completion
    void clearStack(int stack);
    void allocateStack();
    int getNumPages();
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    void setTLB(int vpn);
    void IPTMiss(int vpn, int ppn);
    void swappedPage(int vpn, int swapLocation);
    void evictedPage(int vpn);
    Table fileTable;			// Table of openfiles

 private:
    PageTableEntry *pageTable;	// Assume linear page table translation
					// for now!
    Lock* pageLock; // so multiple threads in the same addrspace do not
                    // mess with the page table at the same time
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    std::queue<int>* numPageQueue;
    Lock* queueLock;
    OpenFile* executable;
};

#endif // ADDRSPACE_H
