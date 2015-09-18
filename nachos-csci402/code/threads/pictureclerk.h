/*
	Class containing all of picture clerk's data and functions
*/
#ifndef PICTURECLERK_H
#define PICTURECLERK_H

#include "clerk.h"

class PictureClerk : public Clerk {
	public:
		PictureClerk(int _id, char* _name);
		void Run(); // the function used to actually run the clerk's processes
	private:
};

#endif
