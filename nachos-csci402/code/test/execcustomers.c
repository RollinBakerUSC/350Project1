#include "syscall.h"

int main() {
	int i = 0;
	for(; i < 7; i++) {
		Exec("../test/customer", 16);
	}
	Exit(0);
}
