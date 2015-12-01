#include "syscall.h"
#include "setup.h"

int main() {
	int i = 0;
	for(; i < NUM_APPCLERKS; i++) {
		Exec("../test/appclerk", 16);
	}
	Exit(0);
}
