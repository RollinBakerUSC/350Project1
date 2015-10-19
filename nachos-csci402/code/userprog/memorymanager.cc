#include "memorymanager.h"
#include "process.h"
#include "system.h"
#include "table.h"
#include <vector.h>


MemoryManager::MemoryManager(){
	bitMap = new BitMap(numPhysPages);
	processTable = new std::vector<Process*>();
}

int MemoryManager::allocatePage(AddrSpace* as){
	int ppn = bitMap->Find();
	return ppn;
}

bool addProcess(AddrSpace* as, TranslationEntry* pt, Thread* t){
	Process* process = new Process(as, pt, t);
	if(process){
		processTable->push_back(process);
		return true;
	}
	else{
		return false;
	}
}

bool MemoryManager::updatePageTable(AddrSpace* as, TranslationEntry* pt){
	for(unsigned int i = 0; i < processTable.size(); i++){
		if(processTable[i].processSpace == as){
			processTable[i].pageTable = pt;
		}
	}	
}

int MemoryManager::freePage(AddrSpace* as, int vpn){

}