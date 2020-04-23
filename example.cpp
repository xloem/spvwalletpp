#include <spvwallet.hpp>

#include <iostream>

using namespace std;

int main()
{
	spvwallet spv(false);
	spvwallet::configuration configuration;
	configuration.dataDirectory = "test-watch";
	configuration.dataDirectory = "/home/karl/.spvwallet";
	configuration.dataDirectory = "test-watch-2";
	spv.start(true, configuration);

	configuration = spv.getconfiguration();
	cout << "Configuration:" << endl;
	cout << "  dataDirectory: " << configuration.dataDirectory << endl;
	cout << "  network: " << configuration.network << endl;
	cout << "  mnemonic: " << configuration.mnemonic << endl;
	cout << "  creationDate: " << configuration.walletCreationDate << endl;
	cout << "  trustedPeer: " << configuration.trustedPeer << endl;
	cout << "  tor: " << configuration.tor << endl;
	cout << "  binary: " << configuration.binary << endl;

	std::vector<std::string> watchedaddresses = {
		"1E3J3gjeRuq9R9GgE96g7BSVmZJNgZMqWc",
		"13eZkQTZCFyEt8Ch3SkX1fubCbK28gzwNx",
		"13VRTkYjm57YVnKcdEnkBoGJw9ToWMKzW3",
		"1217dUx97zxsE2zEYwauj9dZspYP4X2uLQ",
		"12AGwnjLmwCvX4ZbgVR1gDQfBr98RVhCun",
		"1E9NF9DyUrTGR1DgsPAqmjQFzjhxzwtKSj",
		"1Q6yDxD77UfcE1eDsNSAU84rs9avhrSUqu",
		"1D9aKPRquRLzgTymFBgitJfmZf6zU45Q9d"
	};

	for (auto address : watchedaddresses) {
		spv.addwatchedaddress(address);
	}
	//spv.resyncblockchain(1587472633);

	cout << "SPVWallet version: " << spv.version() << endl;
	cout << "Current address: " << spv.currentaddress() << endl;
	auto addresses = spv.listaddresses();
	cout << "Some addresses: " << endl;
	for (int i = 0; i < 8 && i < addresses.size(); ++ i) {
		cout << "  " << addresses[i] << endl;
	}

	spv.waitForSync([](uint64_t current_block, uint64_t total_blocks, uint64_t peers, string message){
		cout << "\r" << message << " " << current_block << "/" << total_blocks << "                " << flush;
	});
	cout << endl;

	cout << "Balance: " << spv.balance() << endl;
	cout << "Transactions:" << endl;
	for (auto t : spv.transactions()) {
		cout << "  " << t.txid << endl;
		cout << "    value: " << t.value << endl;
		cout << "    timestamp: " << t.timestamp << endl;
		cout << "    confirmations: " << t.confirmations << endl;
		cout << "    height: " << t.height << endl;
		cout << "    watchOnly: " << t.watchOnly << endl;
		auto data = spv.raw(t.txid);
		cout << "    size: " << data.size() << endl;
	}
	cout << "Unspent:" << endl;
	for (auto u : spv.unspents()) {
		cout << "  " << u.transaction << ":" << u.output << endl;
		cout << "    value: " << u.value << endl;
		cout << "    script: " << u.scripthex << endl;
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
