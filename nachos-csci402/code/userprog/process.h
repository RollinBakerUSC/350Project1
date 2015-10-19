#ifndef PROCESS_H
#define PROCESS_H

#include "translate.h"
#include "addrspace.h"
#include "synch.h"
#include <vector.h>

class Process{
public:
	Process(AddrSpace* as, TranslationEntry* pt, Thread* t);
	void addThread(Thread* t);
	int removeThread(Thread* t);
	int translation(int vpn);
	bool isSameProcess(AddrSpace* as);
private:
	TranslationEntry* pageTable;
	std::vector<Thread*>* activeThreads;
	AddrSpace* processSpace;

};
#endif

