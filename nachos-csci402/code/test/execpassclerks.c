#include "syscall.h"
#include "setup.h"

int main() {
	int i = 0;
	for(; i < NUM_PASSCLERKS; i++) {
		Exec("../test/passclerk", 17);
	}
	Exit(0);
}
