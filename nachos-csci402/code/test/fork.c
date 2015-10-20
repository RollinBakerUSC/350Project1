#include "syscall.h"

void print1() {
	Print("CSCI 350\n", 9);
	Exit(0);
}

void print2() {
	Print("OS\n", 3);
	Exit(0);
}

int main() {
	Fork((void*)print1, "Print", 5, 1);
	Fork((void*)print2, "Print", 5, 2);
	Exit(0);
}