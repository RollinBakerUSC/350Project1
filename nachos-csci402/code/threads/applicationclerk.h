/*
	Class containing all of application clerk's data and functions
*/
#ifndef APPLICATIONCLERK_H
#define APPLICATIONCLERK_H

#include "clerk.h"

class ApplicationClerk : public Clerk {
	public:
		ApplicationClerk(int _id, char* _name);
		void Run(); // the function used to actually run the clerk's processes
	private:
};

#endif
