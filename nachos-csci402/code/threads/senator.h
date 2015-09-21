/*
	Class containing all of Senators data and functions
*/

#ifndef SENATOR_H
#define SENATOR_H

class Senator {
	public:
		Senator(int _SocialSecurity, char* _name);
		void Run();
		char* getName();

	private:
		int socialSecurity;
		char* name; // for debugging and output purposes
					// naming convention will be "Senator" + socialSecurity
		int money; // always 100

		void goToAppClerk();
		void goToPicClerk();
		void goToPassClerk();
		void goToCashier();
};

#endif
