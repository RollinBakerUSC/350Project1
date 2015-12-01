#include "syscall.h"

int main () {
	int lock1 = CreateLock("Lock", 4, 1);
	int lock2 = CreateLock("Lock", 4, 2);
	int cv1 = CreateCondition("CV", 2, 1);
	int cv2 = CreateCondition("CV", 2, 2);
	int mv1 = CreateMV("MV", 2, 1, 1);
	Exit(0);
}