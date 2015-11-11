#include "syscall.h"

int main () {
	int lock1 = CreateLock("Lock1", 5);
	int lock2 = CreateLock("Lock2", 5);
	int cv1 = CreateCondition("CV1", 3);
	int cv2 = CreateCondition("CV2", 3);
	int mv1 = CreateMV("MV1", 3, 1);
	Exit(0);
}