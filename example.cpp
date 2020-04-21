#include <spvwallet.hpp>

#include <iostream>

using namespace std;

int main()
{
	spvwallet spv;

	cout << "SPVWallet version: " << spv.version() << endl;
	cout << "Current address: " << spv.currentaddress() << endl;
	cout << "Balance: " << spv.balance() << endl;
}
