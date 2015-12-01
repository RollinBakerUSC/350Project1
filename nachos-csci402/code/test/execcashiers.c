#include "syscall.h"
#include "setup.h"

int main() {
	int i = 0;
	for(; i < NUM_CASHIERS; i++) {
		Exec("../test/cashier", 15);
	}
	Exit(0);
}
