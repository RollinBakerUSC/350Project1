#include "syscall.h"

int main() {
	int i = 0;
	for(; i < 5; i++) {
		Exec("../test/customer", 16);
	}
	Exit(0);
}
