#include <spvwallet.hpp>

#include <iostream>

using namespace std;

int main()
{
	spvwallet spv;

	cout << "SPVWallet version: " << spv.version() << endl;
	cout << "Current address: " << spv.currentaddress() << endl;
	cout << "Balance: " << spv.balance() << endl;
	cout << "Transactions:" << endl;
	for (auto t : spv.transactions()) {
		cout << "  " << t.txid << endl;
		cout << "    value: " << t.value << endl;
		cout << "    timestamp: " << t.timestamp << endl;
		cout << "    confirmations: " << t.confirmations << endl;
		cout << "    height: " << t.height << endl;
		cout << "    watchOnly: " << t.watchOnly << endl;
	}
}
