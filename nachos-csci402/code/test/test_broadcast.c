#include "syscall.h"

int main() {
	int lock = 1;
	int cv = 1;
	Acquire(lock);
	Broadcast(cv, lock);
	Release(lock);
	Exit(0);
}