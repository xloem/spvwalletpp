#include <spvwallet.hpp>

#include "utils.hpp"

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

spvwallet::configuration::configuration()
: network(MAIN),
  walletCreationDate(0),
  tor(false),
  binary("spvwallet")
{ }

string spvwallet::command(vector<string> commands, bool output, string return_at_output, unique_ptr<subprocess::Popen> * pointer)
{
	commands.insert(commands.begin(), prefix);
	string result = process(commands, output, return_at_output, pointer);

	// remove trailing whitespace
	do {
		switch(result[result.size()-1]) {
		case '\n':
		case '\r':
			result.resize(result.size()-1);
			continue;
		}
	} while(false);

	// handle errors
	if (result.compare(0, 10, "rpc error:") == 0) {
		error::makeAndThrow(result.substr(10));
	}

	return result; // suspect result may be json or line-delimited
}

void spvwallet::start(bool background, spvwallet::configuration configuration)
{
	if (running()) {
		throw error("Unavailable", "already started");
	}
	vector<string> commands({"start"});
	if (configuration.dataDirectory.size()) {
		commands.push_back("--datadir=" + configuration.dataDirectory);
		repository_path = configuration.dataDirectory;
	}
	switch (configuration.network)
	{
	case spvwallet::configuration::MAIN:
		break;
	case spvwallet::configuration::TEST:
		commands.push_back("--testnet");
		if (repository_path.size()) {
			repository_path += PATH_SEPARATOR + "testnet";
		}
		break;
	case spvwallet::configuration::REGRESSION:
		commands.push_back("--regtest");
		if (repository_path.size()) {
			repository_path += PATH_SEPARATOR + "regtest";
		}
		break;
	default:
		throw error("Unrecognized", "invalid network selection");
	};
	if (configuration.mnemonic.size()) {
		commands.push_back("--mnemonic=" + configuration.mnemonic);
	}
	if (configuration.walletCreationDate) {
		commands.push_back("--walletcreationdate=" + to_iso8601(configuration.walletCreationDate));
	}
	if (configuration.trustedPeer.size()) {
		commands.push_back("--trustedpeer=" + configuration.trustedPeer);
	}
	if (configuration.tor) {
		commands.push_back("--tor");
	}
	//commands.push_back("-v");
	commands.push_back("--feeapi=");
	if (background) {
		command(commands, false, "[Press Ctrl+C to exit]", &this->background); 
	} else {
		command(commands, true);
	}
}

spvwallet::error::error(string code, string description)
: runtime_error(code + ": " + description), code(code), description(description)
{ }

void spvwallet::error::makeAndThrow(string description)
{
	description = " " + description;
	auto codestart = description.find(" code = ");
	auto descstart = description.find(" desc = ");
	auto code = description.substr(codestart + 8, descstart - codestart - 8);
	description = description.substr(descstart + 8);
	if (code == "Unavailable") {
		throw unavailable(code, description);
	} else if (code == "Internal") {
		throw internal(code, description);
	} else {
		throw error(code, description);
	}
}

spvwallet::spvwallet(bool startInBackgroundIfNotRunning, spvwallet::configuration startConfiguration)
: prefix(startConfiguration.binary)
{
	if (startInBackgroundIfNotRunning) {
		if (!running()) {
			start(true, startConfiguration);
		}
	}
}
spvwallet::~spvwallet()
{
	if (background) { stop(); }
}

void spvwallet::stop()
{
	if (background) {
		background->kill(SIGINT);
		background.reset();
	} else {
		command({"stop"});
	}
}

string spvwallet::version()
{
	return command({"version"});
}

uint64_t spvwallet::chaintip()
{
	string chaintip = command({"chaintip"});
	return stoull(chaintip);
}

string spvwallet::currentaddress()
{
	return command({"currentaddress"});
}

uint64_t spvwallet::balance()
{
	auto balances = json::parse(command({"balance"}));
	return balances["confirmed"].get<uint64_t>() + balances["unconfirmed"].get<uint64_t>();
}

void spvwallet::addwatchedaddress(string address)
{
	for (auto & existing : listaddresses()) {
		// if a wallet address is added as a watched address,
		// its future transactions won't be used in calculations
		if (address == existing) { return; }
	}
	try {
		command({"addwatchedscript", address});
	} catch (error::internal &result) {
		if (result.description == "grpc: error while marshaling: proto: Marshal called with nil") { return; }
		throw;
	}
}

vector<string> spvwallet::listaddresses()
{
	auto addresses = command({"listaddresses"});
	return subprocess::util::split(addresses, "\r\n");
}

string spvwallet::getkey(string address)
{
	return command({"getkey", address});
}

vector<spvwallet::transaction> spvwallet::transactions()
{
	vector<transaction> result;
	auto transactions = json::parse(command({"transactions"}));
	for (auto tjson : transactions)
	{
		transaction t;
		t.txid = tjson["txid"].get<string>();
		t.value = tjson["value"].get<uint64_t>();
		auto status = tjson["status"].get<string>();
		if (status == "UNCONFIRMED") {
			t.status = transaction::UNCONFIRMED;
		} else if (status == "PENDING") {
			t.status = transaction::PENDING;
		} else if (status == "CONFIRMED") {
			t.status = transaction::CONFIRMED;
		} else if (status == "STUCK") {
			t.status = transaction::STUCK;
		} else if (status == "DEAD") {
			t.status = transaction::DEAD;
		} else if (status == "ERROR") {
			t.status = transaction::ERROR;
		} else {
			t.status = (decltype(t.status))-1;
		}
		t.timestamp = from_iso8601(tjson["timestamp"].get<string>());
		t.confirmations = tjson["confirmations"].get<uint64_t>();
		t.height = tjson["height"].get<uint64_t>();
		t.watchOnly = tjson["watchOnly"].get<bool>();
		result.emplace_back(t);
	}
	return result;
}

vector<spvwallet::peer> spvwallet::peers()
{
	vector<peer> result;
	auto peers = json::parse(command({"peers"}));
	for (auto pjson : peers)
	{
		peer p;
		p.address = pjson["address"].get<string>();
		p.bytesSent = pjson["bytesSent"].get<uint64_t>();
		p.bytesReceived = pjson["bytesReceived"].get<uint64_t>();
		p.connected = pjson["connected"].get<bool>();
		p.id = pjson["id"].get<uint64_t>();
		p.lastBlock = pjson["lastBlock"].get<uint64_t>();
		p.protocolVersion = pjson["protocolVersion"].get<uint64_t>();
		p.services = pjson["services"].get<string>();
		p.userAgent = pjson["userAgent"].get<string>();
		p.timeConnected = from_iso8601(pjson["timeConnected"].get<string>());
		result.emplace_back(p);
	}
	return result;
}

void spvwallet::resyncblockchain(uint64_t timestamp)
{
	command({"resyncblockchain", to_iso8601(timestamp)});
}

bool spvwallet::running()
{
	try {
		chaintip();
		return true;
	} catch (error::unavailable &) {
		return false;
	}
}
