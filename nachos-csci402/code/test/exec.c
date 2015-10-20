#include "syscall.h"

int main () {
	Exec("../test/print", 13);
	Print("Between.\n", 9);
	Exec("../test/print", 13);
	Exit(0);
}