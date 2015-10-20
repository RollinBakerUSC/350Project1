#include "syscall.h"

int main () {
	int lock1 = CreateLock("Lock1", 5);
	int lock2 = CreateLock("Lock2", 5);
	int lock3 = CreateLock("Lock3", 5);
	int cv1 = CreateCondition("CV1", 3);
	int cv2 = CreateCondition("CV2", 3);
	Print("test ", 5);
	PrintInt(GetID());
	Print(".\n", 2);
	Signal(cv2, lock1);
	DestroyLock(lock2);
	DestroyLock(lock3);
	DestroyLock(lock1);
	DestroyCondition(cv1);
	DestroyCondition(cv2);
}