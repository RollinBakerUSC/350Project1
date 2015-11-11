#include "syscall.h"

int main() {
	int lock = 1;
	int cv = 1;
	Acquire(lock);
	Wait(cv, lock);
	Print("Done!\n", 6);
	Release(lock);
	Exit(0);
}