#include "syscall.h"
#include "setup.h"

int main() {
	int i = 0;
	for(; i < NUM_SENATORS; i++) {
		Exec("../test/senator", 15);
	}
	Exit(0);
}
