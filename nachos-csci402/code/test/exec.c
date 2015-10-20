#include "syscall.h"

int main () {
	Exec("../test/fork", 12);
	Exec("../test/print", 13);
	Exit(0);
}