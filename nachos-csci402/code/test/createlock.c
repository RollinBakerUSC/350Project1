#include "syscall.h"

int main() {
	int s = CreateLock("abc", 3);
	int t = CreateLock("def", 3);
	int u = CreateLock("ghi", 3);
	int v = CreateCondition("xyz", 3);

	Acquire(u);

	DestroyCondition(v);
}