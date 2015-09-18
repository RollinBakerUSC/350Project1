/*
	Class containing all of customer's data and functions
*/

#ifndef CUSTOMER_H
#define CUSTOMER_H

class Customer {
	public:
		Customer(int _socialSecurity, char* _name);
		void Run(); // the function used to actually begin the customer's processes
		char* getName();
	private:
		int socialSecurity;
		int money; // either 100, 600, 1100, 1600
		char* name; // for debugging and output purposes
					// naming convention will be "Customer" + socialSecurity
};

#endif
