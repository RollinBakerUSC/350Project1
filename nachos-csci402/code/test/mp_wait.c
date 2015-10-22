#include "syscall.h"

int lock;
int cv;


void dummy(){
	Acquire(lock);
	Wait(cv, lock);
	Print("ERROR\n", 6);
	Exit(0);
}


int main () {

	lock = CreateLock("Lock", 4);
	cv = CreateCondition("CV", 3);
	
	Fork((void*)dummy, "increment", 9, 1);
	Fork((void*)dummy, "increment", 9, 2);
	Fork((void*)dummy, "increment", 9, 3);
	Exit(0);


}