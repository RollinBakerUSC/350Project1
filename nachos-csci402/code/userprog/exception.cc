// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

void Exec_Thread() {
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  machine->Run();
  ASSERT(false);
}

int Exec_Syscall(unsigned int vaddr, int size) {
  processLock->Acquire();
  char* buf = new char[size+1];
  if(!buf) {
    return -1;
  }
  if(copyin(vaddr, size, buf) == -1) {
    printf("Unable to read file name for Exec.\n");
    delete buf;
    return -1;
  }
  buf[size] = '\0';

  OpenFile* executable = fileSystem->Open(buf);

  if(executable == NULL) {
    printf("Unable to open file - %s - for Exec.\n", buf);
    delete buf;
    return -1;
  }

  AddrSpace* space = new AddrSpace(executable);

  Process* newProcess = new Process(space);
  processTable->push_back(newProcess);
  processLock->Release();

  Thread* t = new Thread(buf);
  t->space = space;
  
  unsigned int stack = space->getNumPages()*PageSize - 16;
  std::pair<Thread*, unsigned int>* newPair = new std::pair<Thread*, unsigned int>;
  newPair->first = t;
  newPair->second = stack;
  processLock->Acquire();
  for(unsigned int i = 0; i < processTable->size(); i++) {
    if(space == processTable->at(i)->space) {
      processTable->at(i)->threadStacks->push_back(newPair);
      break;
    }
  }
  processLock->Release();

  delete executable;

  t->Fork((VoidFunctionPtr)Exec_Thread, 0);

  return 0;
}

void Fork_Thread(unsigned int vaddr) {
  unsigned int stack = currentThread->space->getNumPages()*PageSize - 16;
  std::pair<Thread*, unsigned int>* newPair = new std::pair<Thread*, unsigned int>;
  newPair->first = currentThread;
  newPair->second = stack;
  processLock->Acquire();
  for(unsigned int i = 0; i < processTable->size(); i++) {
    if(currentThread->space == processTable->at(i)->space) {
      processTable->at(i)->threadStacks->push_back(newPair);
      break;
    }
  }
  processLock->Release();
  currentThread->space->InitRegisters();
  currentThread->space->RestoreState();
  machine->WriteRegister(PCReg, vaddr);
  machine->WriteRegister(NextPCReg, vaddr+4);
  machine->WriteRegister(StackReg, stack);
  machine->Run();
}

void Fork_Syscall(unsigned int addr, unsigned int vaddr, int size, int id) {
  // first update the process table
  processLock->Acquire();
  for(unsigned int i = 0; i < processTable->size(); i++) {
    if(currentThread->space == processTable->at(i)->space) {
      processTable->at(i)->numThreads++;
      break;
    }
  }
  processLock->Release();

  char* buf = new char[size+3];
  copyin(vaddr, size, buf);
  buf[size] = ' ';
  buf[size+1] = (char)(id+48);
  buf[size+2] = '\0';

  Thread* t = new Thread(buf);
  currentThread->space->allocateStack();
  t->space = currentThread->space;

  t->Fork((VoidFunctionPtr)Fork_Thread, addr);
}

int CreateLock_Syscall(unsigned int vaddr, int size) {
  char* buf = new char[size+1];

  if(!buf) {
    return -1;
  }
  if(copyin(vaddr, size, buf) == -1) {
    printf("Bad pointer passed to CreateLock.\n");
    delete buf;
    return -1;
  }

  buf[size] = '\0';

  KernelLock* newLock = new KernelLock(buf, currentThread->space);
  kernelLockLock->Acquire();
  kernelLockTable->push_back(newLock);
  int toReturn = kernelLockTable->size()-1;
  kernelLockLock->Release();
  return toReturn;
}

void DestroyLock_Syscall(unsigned int index) {
  kernelLockLock->Acquire();
  if(index < kernelLockTable->size()) {
    KernelLock* lock = kernelLockTable->at(index);
    if(currentThread->space == lock->addrspace && lock->valid) {
      if(lock->numThreads == 0) {
        lock->valid = false;
        lock->isToBeDeleted = false;
        lock->addrspace = NULL;
        delete lock->lock;
        lock->lock = NULL;
      }
      else {
        lock->isToBeDeleted = true;
      }
    }
  }
  kernelLockLock->Release();
}

void Acquire_Syscall(unsigned int index) {
  kernelLockLock->Acquire();
  if(index < kernelLockTable->size()) {
    KernelLock* lock = kernelLockTable->at(index);
    if(currentThread->space == lock->addrspace && lock->valid) {
      lock->numThreads++;
      lock->lock->Acquire();
    }
  }
  kernelLockLock->Release();
}

void Release_Syscall(unsigned int index) {
  kernelLockLock->Acquire();
  if(index < kernelLockTable->size()) {
    KernelLock* lock = kernelLockTable->at(index);
    if(currentThread->space == lock->addrspace && lock->valid) {
      lock->numThreads--;
      lock->lock->Release();
      /* check if the lock has no one waiting for it and is to be deleted */
      if(lock->numThreads == 0 && lock->isToBeDeleted) {
        lock->valid = false;
        lock->isToBeDeleted = false;
        lock->addrspace = NULL;
        delete lock->lock;
        lock->lock = NULL;
      }
    }
  }
  kernelLockLock->Release();
}

int CreateCondition_Syscall(unsigned int vaddr, int size) {
  char* buf = new char[size+1];

  if(!buf) {
    return -1;
  }
  if(copyin(vaddr, size, buf) == -1) {
    printf("Bad pointer passed to CreateCondition.\n");
    delete buf;
    return -1;
  }

  buf[size] = '\0';

  KernelCondition* newCondition = new KernelCondition(buf, currentThread->space);
  kernelCVLock->Acquire();
  kernelCVTable->push_back(newCondition);
  int toReturn = kernelCVTable->size()-1;
  kernelCVLock->Release();
  return toReturn;
}

void DestroyCondition_Syscall(unsigned int index) {
  kernelCVLock->Acquire();
  if(index < kernelCVTable->size()) {
    KernelCondition* cv = kernelCVTable->at(index);
    if(currentThread->space == cv->addrspace && cv->valid) {
      if(cv->numThreads == 0) {
        cv->valid = false;
        cv->isToBeDeleted = false;
        cv->addrspace = NULL;
        delete cv->condition;
        cv->condition = NULL;
      }
      else {
        cv->isToBeDeleted = true;
      }
    }
  }
  kernelCVLock->Release();
}

void Wait_Syscall(unsigned int cvIndex, unsigned int lockIndex) {
  kernelCVLock->Acquire();
  kernelLockLock->Acquire();
  if(cvIndex < kernelCVTable->size() && lockIndex < kernelLockTable->size()) {
    KernelCondition* cv = kernelCVTable->at(cvIndex);
    KernelLock* lock = kernelLockTable->at(lockIndex);
    if(currentThread->space == cv->addrspace &&
      currentThread->space == lock->addrspace &&
      cv->valid && lock->valid) {
      cv->numThreads++;
      cv->condition->Wait(lock->lock);
      cv->numThreads--;
    }
  }
  kernelCVLock->Release();
  kernelLockLock->Release();
}

void Signal_Syscall(unsigned int cvIndex, unsigned int lockIndex) {
  kernelCVLock->Acquire();
  kernelLockLock->Acquire();
  if(cvIndex < kernelCVTable->size() && lockIndex < kernelLockTable->size()) {
    KernelCondition* cv = kernelCVTable->at(cvIndex);
    KernelLock* lock = kernelLockTable->at(lockIndex);
    if(currentThread->space == cv->addrspace &&
      currentThread->space == lock->addrspace &&
      cv->valid && lock->valid) {
      cv->condition->Signal(lock->lock);
    }
  }
  kernelCVLock->Release();
  kernelLockLock->Release();
}

void Broadcast_Syscall(unsigned int cvIndex, unsigned int lockIndex) {
  kernelCVLock->Acquire();
  kernelLockLock->Acquire();
  if(cvIndex < kernelCVTable->size() && lockIndex < kernelLockTable->size()) {
    KernelCondition* cv = kernelCVTable->at(cvIndex);
    KernelLock* lock = kernelLockTable->at(lockIndex);
    if(currentThread->space == cv->addrspace &&
      currentThread->space == lock->addrspace &&
      cv->valid && lock->valid) {
      cv->condition->Broadcast(lock->lock);
    }
  }
  kernelCVLock->Release();
  kernelLockLock->Release();
}

void Print_Syscall(unsigned int vaddr, int size) {
  char* buf = new char[size+1];

  copyin(vaddr, size, buf);

  buf[size] = '\0';

  cout << buf;
}

void PrintInt_Syscall(int toPrint) {
  cout << toPrint;
}

/* used solely for the passport office */
int GetID_Syscall() {
  string name = currentThread->getName();
  char c = name[name.length()-1];
  int id = c - '0';
  return id;
}

void Exit_Syscall(int status) {
  processLock->Acquire();
  Process* myProcess;
  int myIndex;
  // find your process in the process table
  for(unsigned int i = 0; i < processTable->size(); i++) {
    if(currentThread->space == processTable->at(i)->space) {
      myProcess = processTable->at(i);
      myIndex = i;
      break;
    }
  }
  // thread who called exit is not the last thread in the process
  if(myProcess->numThreads > 1) {
    int threadIndex;
    myProcess->numThreads--;
    std::vector<std::pair<Thread*, unsigned int>* >* v = myProcess->threadStacks;
    std::pair<Thread*, unsigned int>* p;
    for(unsigned int i = 0; i < v->size(); i++) {
      if(currentThread == v->at(i)->first) {
        threadIndex = i;
        p = v->at(i);
        break;
      }
    }
    myProcess->space->clearStack(p->second);
    delete p;
    myProcess->threadStacks->erase(myProcess->threadStacks->begin() + threadIndex);
    processLock->Release();
    currentThread->Finish();
  }
  // else this thread is the last thread of a process
  else { 
    // if this is the last thread of the last process
    if(processTable->size() == 1) {
      interrupt->Halt();
    }
    // else the last thread of some process
    else {
      myProcess->space->clearMem();
      kernelLockLock->Acquire();
      for(unsigned int i = 0; i < kernelLockTable->size(); i++) {
        if(kernelLockTable->at(i)->addrspace == myProcess->space) {
          DestroyLock_Syscall(i);
        }
      }
      kernelLockLock->Release();
      kernelCVLock->Acquire();
      for(unsigned int i = 0; i < kernelCVTable->size(); i++) {
        if(kernelCVTable->at(i)->addrspace == myProcess->space) {
          DestroyCondition_Syscall(i);
        }
      }
      kernelCVLock->Release();
      delete myProcess->space;
      processTable->erase(processTable->begin() + myIndex);
      delete myProcess;
      processLock->Release();
      currentThread->Finish();
    }
  }
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
      case SC_Exit:
    DEBUG('a', "Exit syscall.\n");
    Exit_Syscall(machine->ReadRegister(4));
    break;
      case SC_Exec:
    DEBUG('a', "Exec syscall.\n");
    rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
      case SC_Fork:
    DEBUG('a', "Fork syscall.\n");
    Fork_Syscall(machine->ReadRegister(4),
            machine->ReadRegister(5),
            machine->ReadRegister(6),
            machine->ReadRegister(7));
    break;
      case SC_Yield:
    DEBUG('a', "Yield Syscall.\n");
    currentThread->Yield();
    break;
      case SC_CreateLock:
    DEBUG('a', "CreateLock syscall.\n");
    rv = CreateLock_Syscall(machine->ReadRegister(4),
                machine->ReadRegister(5));
    break;
      case SC_DestroyLock:
    DEBUG('a', "DestroyLock syscall.\n");
    DestroyLock_Syscall(machine->ReadRegister(4));
    break;
      case SC_Acquire:
    DEBUG('a', "Acquire syscall.\n");
    Acquire_Syscall(machine->ReadRegister(4));
    break;
      case SC_Release:
    DEBUG('a', "Release syscall.\n");
    Release_Syscall(machine->ReadRegister(4));
    break;
      case SC_CreateCondition:
    DEBUG('a', "CreateCondition syscall.\n");
    rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_DestroyCondition:
    DEBUG('a', "DestroyCondition syscall.\n");
    DestroyCondition_Syscall(machine->ReadRegister(4));
    break;
      case SC_Wait:
    DEBUG('a', "Wait syscall.\n");
    Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Signal:
    DEBUG('a', "Signal syscall.\n");
    Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Broadcast:
    DEBUG('a', "Broadcast syscall.\n");
    Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Print:
    DEBUG('a', "Print syscall.\n");
    Print_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_PrintInt:
    DEBUG('a', "PrintInt syscall.\n");
    PrintInt_Syscall(machine->ReadRegister(4));
    break;
      case SC_GetID:
    DEBUG('a', "GetID syscall.\n");
    rv = GetID_Syscall();
    break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
