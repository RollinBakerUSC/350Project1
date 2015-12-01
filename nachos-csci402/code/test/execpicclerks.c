#include "syscall.h"
#include "setup.h"

int main() {
	int i = 0;
	for(; i < NUM_PICCLERKS; i++) {
		Exec("../test/picclerk", 16);
	}
	Exit(0);
}
