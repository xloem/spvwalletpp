#include <spvwallet.hpp>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

spvwallet::configuration::configuration()
: network(BITCOIN), mnemonicDate(0), tor(false)
{ }

#include <time.h>
uint64_t from_iso8601(string datetime)
{
	struct tm tm;
	strptime(datetime.c_str(), "%FT%T%z", &tm);
	return mktime(&tm);
}
string to_iso8601(uint64_t timestamp)
{
	time_t timestamp_proper = timestamp;
	struct tm * tm_p = gmtime(&timestamp_proper);
	static thread_local char buffer[1024];
	strftime(buffer, sizeof(buffer), "%FT%T%z", tm_p);
	return buffer;
}

#include <string>
#include <subprocess.hpp>
//#include <iomanip>
#include <iostream>
string process(std::vector<string> const & commands, bool output = false, string return_at_output = {}, pid_t * pid_pointer = 0)
{
	auto process = subprocess::Popen(commands, output ? subprocess::output{stdout} : subprocess::output{subprocess::PIPE}, output ? subprocess::error{stderr} : subprocess::error{subprocess::PIPE});
	if (pid_pointer) { *pid_pointer = process.pid(); }
	if (return_at_output.size() == 0) {
		process.wait();
	} else {
		string result;
		size_t idx = 0;
		char buffer[256] = { 0 };
		while (result.find(return_at_output, idx) == string::npos) {
			idx = result.size() - return_at_output.size() + 1;
			if (0 == fgets(buffer, sizeof(buffer), process.output())) {
				// throw error maybe?
				break;
			}
			
			result.append(buffer);
			if (output) {
				cout << buffer;
			}
		}
		if (output) {
			cout << endl;
		}
		return result;
	}
	auto results = process.communicate();
	return string(results.second.buf.data(), results.second.length) + string(results.first.buf.data(), results.first.length);
}

string spvwallet::command(vector<string> commands, bool output, string return_at_output, uint64_t * pid_pointer)
{
	commands.insert(commands.begin(), prefix);
	string result = process(commands, output, return_at_output, (pid_t*)pid_pointer);

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

#include <unistd.h>
void spvwallet::start(bool background, spvwallet::configuration configuration)
{
	std::vector<string> commands({"start"});
	if (configuration.dataDirectory.size()) {
		commands.push_back("--datadir=" + configuration.dataDirectory);
	}
	switch (configuration.network)
	{
	case spvwallet::configuration::BITCOIN:
		break;
	case spvwallet::configuration::TESTNET:
		commands.push_back("--testnet");
		break;
	case spvwallet::configuration::REGTEST:
		commands.push_back("--regtest");
		break;
	default:
		throw error("Unrecognized", "invalid network selection");
	};
	if (configuration.mnemonic.size()) {
		commands.push_back("--mnemonic=" + configuration.mnemonic);
		if (configuration.mnemonicDate) {
			commands.push_back("--walletcreationdate=" + to_iso8601(configuration.mnemonicDate));
		}
	}
	if (configuration.trustedPeer.size()) {
		commands.push_back("--trustedpeer=" + configuration.trustedPeer);
	}
	if (configuration.tor) {
		commands.push_back("--tor");
	}
	commands.push_back("--feeapi=");
	if (background) {
		command(commands, false, "[Press Ctrl+C to exit]", &pid); 
	} else {
		command(commands, true);
	}
}

spvwallet::error::error(string code, string description)
: runtime_error(code + ": " + description)
{ }

void spvwallet::error::makeAndThrow(string description)
{
	description = " " + description;
	auto codestart = description.find(" code = ");
	auto descstart = description.find(" desc = ");
	auto code = description.substr(codestart + 8, descstart - codestart - 8);
	auto desc = description.substr(descstart + 8);
	if (code == "Unavailable") {
		throw unavailable(code, description);
	} else {
		throw error(code, description);
	}
}

spvwallet::spvwallet(std::string path, bool startInBackgroundIfNotRunning, spvwallet::configuration startConfiguration)
: prefix(path), pid(0)
{
	if (startInBackgroundIfNotRunning) {
		try {
			currentaddress();
		} catch (error::unavailable &) {
			start(true, startConfiguration);
		}
	}
}
spvwallet::~spvwallet()
{
	if (pid) { stop(); }
}

void spvwallet::stop()
{
	if (pid) {
		::kill(pid, SIGINT);
		pid = 0;
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
	return std::stoull(chaintip);
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

std::vector<spvwallet::transaction> spvwallet::transactions()
{
	std::vector<transaction> result;
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

std::vector<spvwallet::peer> spvwallet::peers()
{
	std::vector<peer> result;
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
