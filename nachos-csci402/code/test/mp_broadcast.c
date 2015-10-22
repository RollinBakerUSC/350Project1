#include "syscall.h"

int lock;
int cv;
int i;


void dummy1(){
	Acquire(lock);

	i = 1;
	Wait(cv, lock);
	Print("Broadcast Success!\n", 19);
	Release(lock);
	Exit(0);
}

void dummy2(){
	Acquire(lock);

	i = 2;
	Wait(cv, lock);
	Print("Broadcast Success!\n", 19);
	Release(lock);
	Exit(0);
}

void dummy3(){
	Acquire(lock);

	i = 3;
	Wait(cv, lock);
	Print("Broadcast Success!\n", 19);
	Release(lock);
	Exit(0);
}


int main () {
	lock = CreateLock("Lock", 4);
	cv = CreateCondition("CV", 3);
	i = 0;
	Acquire(lock);
	Fork((void*)dummy1, "dummy1", 6, 1);
	Fork((void*)dummy2, "dummy2", 6, 2);
	Fork((void*)dummy3, "dummy3", 6, 3);
	Release(lock);
	while(i != 3){
		Yield();
	}
	Acquire(lock);
	Broadcast(cv, lock);
	Release(lock);
	Exit(0);


}
