#include "syscall.h"

int lock;
int cv;
int i;


void dummy(){
	Acquire(lock);
	Signal(cv, lock);
	Release(lock);
	Exit(0);
}


int main () {
	lock = CreateLock("Lock", 4);
	cv = CreateCondition("CV", 3);
	Acquire(lock);
	Fork((void*)dummy, "increment", 9, 1);
	Wait(cv, lock);
	Print("Signal Success!\n", 16);
	Exit(0);


}
