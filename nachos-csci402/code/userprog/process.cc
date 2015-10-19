#include "process.h"
#include "synch.h"
//include translate.h
#include "addrspace.h"
#include <vector.h>



Process::Process(AddrSpace* as, TranslationEntry* pt, Thread* t){
	pageTable = pt;
	processSpace = as;
	activeThreads = new std::vector<Thread*>();
	activeThreads->push_back(t);
}

void Process::addThread(Thread* t){
	//Do a validation check
	activeThreads->push_back(t);
}

int Process::removeThread(Thread* t){
	for(std::vector<Thread*>::iterator it = activeThreads->begin(); it != activeThreads->end(); ++it){
		if(t == (*it)){
			activeThreads->erase(it);
			return 1;
		}
	}
	return 0;
}

int Process::translation(int vpn){
	return pageTable[vpn].physicalPage;
}

bool Process::isSameProcess(AddrSpace* as){
	return (as == processSpace);
}
