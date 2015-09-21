/*
Class containing all of cashier's data and functions
*/
#ifndef CASHIER_H
#define CASHIER_H

#include "clerk.h"

class Cashier : public Clerk {
public:
	Cashier(int _id, char* _name);
	void Run(); // the function used to actually run the clerk's processes
private:
};

#endif
