/*
Class containing all of passport clerk's data and functions
*/
#ifndef PASSPORTCLERK_H
#define PASSPORTCLERK_H

#include "clerk.h"

class PassportClerk : public Clerk {
public:
	PassportClerk(int _id, char* _name);
	void Run(); // the function used to actually run the clerk's processes
private:
};

#endif
