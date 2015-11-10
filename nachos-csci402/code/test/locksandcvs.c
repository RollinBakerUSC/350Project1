#include "syscall.h"

int main () {
	int lock1 = CreateLock("Lock1", 5);
	int lock2 = CreateLock("Lock2", 5);
	int lock3 = CreateLock("Lock3", 5);
	int lock4;
	PrintInt(lock1);
	PrintInt(lock2);
	PrintInt(lock3);
	Acquire(lock1);
	Acquire(lock2);
	DestroyLock(lock1);
	Acquire(lock3);
	Release(lock1);
	Release(lock2);
	Release(lock3);
	Release(lock2);
	/*int cv1 = CreateCondition("CV1", 3);
	int cv2 = CreateCondition("CV2", 3);
	Print("test ", 5);
	PrintInt(GetID());
	Print(".\n", 2);
	Signal(cv2, lock1);
	DestroyCondition(cv1);
	DestroyCondition(cv2);*/
	DestroyLock(lock2);
	DestroyLock(lock3);
	lock4 = CreateLock("Lock4", 5);
	PrintInt(lock4);
	Exit(0);
}