#include "syscall.h"


void print1(){
	PrintInt(1);
	Exit(0);
}

void print2(){
	PrintInt(2);
	Print("\n", 1);
	Exit(0);
}

int main() {

	int lock1 = CreateLock("Lock1", 5);

	int i;
	for(i = 0; i < 10; i++){
		Acquire(lock1);
		Fork((void*)print1, "Print", 5, 1);
		Release(lock1);

		Acquire(lock1);
		Fork((void*)print2, "Increment", 9, 2);
		Release(lock1);
	}
	DestroyLock(lock1);
	Exit(0);
}