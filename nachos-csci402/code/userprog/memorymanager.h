//

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H


#include "process.h"
#include "addrspace.h"
#include "synch.h"
#include <stdio.h>
#include <vector.h>


class MemoryManager{
public:
	MemoryManager();
	int allocatePage(AddrSpace* mySpace);
	int freePage(AddrSpace* mySpace, int vpn);
	void addProcess(AddrSpace* as, TranslationEntry* pt, Thread* t);
	//remove thread
	//add thread
	
private:
	//Array to manage all of the page tables of all active processes
	std::vector<Process*> processTable;
	BitMap* bitMap;

};

#endif
