#include <spvwallet.hpp>

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main()
{
	spvwallet spv;

	cout << "SPVWallet version: " << spv.version() << endl;
	cout << "Current address: " << spv.currentaddress() << endl;
	cout << "Balance: " << spv.balance() << endl;
	uint64_t chaintip;
	uint64_t maxtip = 0;
	std::vector<spvwallet::peer> peers;
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		chaintip = spv.chaintip();
		peers = spv.peers();
		for (auto & peer : peers) {
			if (peer.lastBlock > maxtip) {
				maxtip = peer.lastBlock;
			}
		}
		cout << "\rChain tip: " << chaintip << " / " << maxtip << " (" << peers.size() << " peers)  " << flush;
	} while (maxtip > chaintip || peers.size() == 0);
	cout << endl;
	cout << "Transactions:" << endl;
	for (auto t : spv.transactions()) {
		cout << "  " << t.txid << endl;
		cout << "    value: " << t.value << endl;
		cout << "    timestamp: " << t.timestamp << endl;
		cout << "    confirmations: " << t.confirmations << endl;
		cout << "    height: " << t.height << endl;
		cout << "    watchOnly: " << t.watchOnly << endl;
	}
	/*
	cout << "Peers:" << endl;
	for (auto p : peers) {
		cout << "  " << p.address << endl;
		cout << "    bytesSent: " << p.bytesSent << endl;
		cout << "    bytesReceived: " << p.bytesReceived << endl;
		cout << "    connected: " << p.connected << endl;
		cout << "    id: " << p.id << endl;
		cout << "    lastBlock: " << p.lastBlock << endl;
		cout << "    protocolVersion: " << p.protocolVersion << endl;
		cout << "    services: " << p.services << endl;
		cout << "    userAgent: " << p.userAgent << endl;
		cout << "    timeConnected: " << p.timeConnected << endl;
	}
	*/
}
