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

void fork_thread(int vaddr /* */) {
	currentThread->space->InitRegisters(); //
	//int regValue = machine->ReadRegister(4); // get reg 4 value
	machine->WriteRegister(PCReg, vaddr);
	machine->WriteRegister(NextPCReg, vaddr + 4);
	//machine->WriteRegister(StackReg, currentThread->space->getNumPages * PageSize - 16); // error here
	currentThread->space->RestoreState();
	machine->Run();
}

void Fork_Syscall(int vaddr /* */) {
	Thread *t = new Thread("Forked thread"); // create a new thread
  currentThread->space->AllocateStack();
	t->space = currentThread->space; // give it space same as parent's


	//int regValue = machine->ReadRegister(4);
	t->Fork((VoidFunctionPtr)fork_thread, vaddr); // thread Fork takes in voinfuncptr and int
}

void exec_thread(int vaddr) {
	//machine->WriteRegister(PCReg, 0);
	//machine->WriteRegister(NextPCReg, 4);
	//machine->WriteRegister(StackReg, currentThread->space->getNumPages * PageSize - 16);
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
}

void Exec_Syscall(char* name, int vaddr) {
	Thread *t = new Thread("Exec thread"); // create a new thread
	OpenFile *executable = fileSystem->Open(name);
   	AddrSpace *space;

    	if (executable == NULL) {
		printf("Unable to open file %s\n", name);
		return;
   	}
   
   	space = new AddrSpace(executable);

   	currentThread->space = space;

   	delete executable;			// close file

   	space->InitRegisters();		// set the initial register values
   	space->RestoreState();		// load page table register

   	t->Fork(exec_thread, vaddr); // 2nd argument needs to be changed. not sure
   	// ASSERT(FALSE);
}

void Exit_Syscall(int status) {
	// not the last executing thread calls Exit
	
	// the last executing thread in last process calls Exit
	
	// the last executing thread in nonlast process calls Exit
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

int CreateLock_Syscall(unsigned int vaddr, int length) {
  char* buf = new char[length+1];

  if(!buf) return -1;

  if( copyin(vaddr,length,buf) == -1 ) {
    printf("%s","Bad pointer passed to CreateLock\n");
    delete buf;
    return -1;
  }

  buf[length] = '\0';

  printf("Creating lock %s.\n", buf);

  KernelLock* newLock = new KernelLock(buf);
  kernelLockTable->push_back(newLock);

  return kernelLockTable->size()-1;
}

void Acquire_Syscall(int index) {
  if(index >= 0 && static_cast<unsigned int>(index) < kernelLockTable->size()) {
    printf("Acquiring lock at index %d.\n", index);
    KernelLock* toAcquire = kernelLockTable->at(index);
    toAcquire->lock->Acquire();
  }
}

void Release_Syscall(int index) {
  if(index >= 0 && static_cast<unsigned int>(index) < kernelLockTable->size()) {
    printf("Releasing lock at index %d.\n", index);
    KernelLock* toRelease = kernelLockTable->at(index);
    toRelease->lock->Release();
  }
}

void DestroyLock_Syscall(int index) {
  if(index >= 0 && static_cast<unsigned int>(index) < kernelLockTable->size()) {
    printf("Destroying lock at index %d.\n", index);

  }
}

int CreateCondition_Syscall(unsigned int vaddr, int length) {
  char* buf = new char[length+1];

  if(!buf) return -1;

  if( copyin(vaddr,length,buf) == -1 ) {
    printf("%s","Bad pointer passed to CreateCondition\n");
    delete buf;
    return -1;
  }

  buf[length] = '\0';

  printf("Creating condition %s.\n", buf);

  KernelCondition* newCondition = new KernelCondition(buf);
  kernelConditionTable->push_back(newCondition);

  return kernelConditionTable->size()-1;
}

void Wait_Syscall(int lockIndex, int conditionIndex) {
  if(lockIndex < 0 || static_cast<unsigned int>(lockIndex) >= kernelLockTable->size()) {
    return;
  }
  if(conditionIndex < 0 || static_cast<unsigned int>(conditionIndex) >= kernelConditionTable->size()) {
    return;
  }
  printf("Waiting on condition at index %d.\n", conditionIndex);
  KernelCondition* c = kernelConditionTable->at(conditionIndex);
  KernelLock* l = kernelLockTable->at(lockIndex);

  c->condition->Wait(l->lock);
}

void Signal_Syscall(int lockIndex, int conditionIndex) {
  if(lockIndex < 0 || static_cast<unsigned int>(lockIndex) >= kernelLockTable->size()) {
    return;
  }
  if(conditionIndex < 0 || static_cast<unsigned int>(conditionIndex) >= kernelConditionTable->size()) {
    return;
  }
  printf("Signalling on condition at index %d.\n", conditionIndex);
  KernelCondition* c = kernelConditionTable->at(conditionIndex);
  KernelLock* l = kernelLockTable->at(lockIndex);

  c->condition->Signal(l->lock);
}

void Broadcast_Syscall(int lockIndex, int conditionIndex) {
  if(lockIndex < 0 || static_cast<unsigned int>(lockIndex) >= kernelLockTable->size()) {
    return;
  }
  if(conditionIndex < 0 || static_cast<unsigned int>(conditionIndex) >= kernelConditionTable->size()) {
    return;
  }
  printf("Broadcasting on condition at index %d.\n", conditionIndex);
  KernelCondition* c = kernelConditionTable->at(conditionIndex);
  KernelLock* l = kernelLockTable->at(lockIndex);

  c->condition->Broadcast(l->lock);
}

void DestroyCondition_Syscall(int index) {
  if(index >= 0 && static_cast<unsigned int>(index) < kernelConditionTable->size()) {
    printf("Destroying condition at index %d.\n", index);

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
      case SC_Yield:
    DEBUG('a', "Yield syscall.\n");
    currentThread->Yield();
    break;
      case SC_CreateLock:
    DEBUG('a', "Create Lock syscall.\n");
    rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    break;
      case SC_Acquire:
    DEBUG('a', "Acquire syscall.\n");
    Acquire_Syscall(machine->ReadRegister(4));
    break;
      case SC_Release:
    DEBUG('a', "Release syscall.\n");
    Release_Syscall(machine->ReadRegister(4));
    break;
      case SC_DestroyLock:
    DEBUG('a', "Destroy Lock syscall.\n");
    DestroyLock_Syscall(machine->ReadRegister(4));
    break;
      case SC_CreateCondition:
    DEBUG('a', "Create Condition syscall.\n");
    rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
      case SC_DestroyCondition:
    DEBUG('a', "Destroy Condition syscall.\n");
    DestroyCondition_Syscall(machine->ReadRegister(4));
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
