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
#include <iomanip>
#include <iostream>
string process(std::vector<string> const & commands, bool output = false, bool wait = true, FILE ** stream_pointer = 0)
{
	string result;
	char buffer[1024];
	std::string command_string;
	for (auto command : commands) {
		size_t position = 0;
		while ((position = command.find("'", position)) != string::npos) {
			command.replace(position, 1, "'\"'\"'");
			position += 5;
		}
		command_string += "'" + command + "' ";
	}
	command_string += "2>&1";
	FILE *stream = popen(command_string.c_str(), "r");
	if (stream_pointer) { *stream_pointer = stream; }
	while (fgets(buffer, sizeof(buffer), stream) != 0) {
		if (output) {
			cerr << buffer;
		}
		result.append(buffer);
		if (!wait) { return result; }
	}
	pclose(stream);
	return result;
}

string spvwallet::command(vector<string> commands, bool output, bool wait, void ** stream_pointer)
{
	commands.insert(commands.begin(), prefix);
	string result = process(commands, output, wait, (FILE**)stream_pointer);

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
		command(commands, false, false); 
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
: prefix(path)
{
	if (startInBackgroundIfNotRunning) {
		try {
			currentaddress();
		} catch (error::unavailable &) {
			start(true, startConfiguration);
		}
	}
}

string spvwallet::version()
{
	return command({"version"});
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
		t.timestamp = from_iso8601(tjson["timestamp"].get<string>());
		t.confirmations = tjson["confirmations"].get<uint64_t>();
		t.height = tjson["height"].get<uint64_t>();
		t.watchOnly = tjson["watchOnly"].get<bool>();
		result.emplace_back(t);
	}
	return result;
}
