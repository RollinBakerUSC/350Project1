#include "syscall.h"

int main() {
	int i;
	int lock = 0;
	int cv = 0;
	int mv;
	Acquire(lock);
	mv = GetMV(0, 0);
	SetMV(0, 0, mv+1);
	Signal(cv, lock);
	Wait(cv, lock);
	Print("I have lock\n", 12);
	Print("MV is ", 6);
	PrintInt(mv);
	Print("\n", 1);
	for(i = 0; i < 100; i++) {
		Yield();
	}
	Print("I am releasing lock\n", 20);
	if(mv == 2) {
		Signal(cv, lock);
	}
	Release(lock);
	Exit(0);
}