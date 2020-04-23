#include <spvwallet.hpp>

#include "utils.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <nlohmann/json.hpp>

#include <fstream>

using namespace std;

SQLite::Database & spvwallet::database()
{
	if (!_database) {
		_database.reset(new spvwallet::database_struct{{repository_path + PATH_SEPARATOR + "wallet.db"}});
	}
	return _database->database;
}

spvwallet::configuration spvwallet::getconfiguration()
{
	configuration result;
	string testnet_tail = PATH_SEPARATOR + "testnet";
	string regtest_tail = PATH_SEPARATOR + "regtest";
	auto repository_path_shrunk = repository_path;
	result.dataDirectory = repository_path;
	result.network = configuration::MAIN;
	if (repository_path.size() > testnet_tail.size()) {
		repository_path_shrunk.resize(repository_path.size() - testnet_tail.size());
		if (repository_path_shrunk + testnet_tail == repository_path) {
			result.dataDirectory = repository_path_shrunk;
			result.network = configuration::TEST;
		} else if (repository_path_shrunk + regtest_tail == repository_path) {
			result.dataDirectory = repository_path_shrunk;
			result.network = configuration::REGRESSION;
		}
	}

	auto & database = this->database();
	result.mnemonic = database.execAndGet("SELECT value FROM config WHERE key = 'mnemonic'").getString();
	string creationDate = database.execAndGet("SELECT value FROM config WHERE key = 'creationDate'").getString();
	result.walletCreationDate = from_iso8601(creationDate);

	ifstream settings_stream(repository_path + PATH_SEPARATOR + "settings.json");
	nlohmann::json settings;
	settings_stream >> settings;

	result.trustedPeer = settings["trustedPeer"].get<string>();
	result.tor = settings["proxy"].get<string>().size();
	
	result.binary = prefix;

	return result;
}

string spvwallet::raw(string transaction)
{
	auto & database = this->database();
	SQLite::Statement query(database, "SELECT tx FROM txns WHERE txid = :transaction LIMIT 1");
	query.reset();
	query.bind(1, transaction);
	query.executeStep();
	return query.getColumn(0).getString();
}

/*
void spvwallet::broadcast(string transaction, string raw)
{
	auto & database = this->database();
	SQLite::Statement insert_stxos(database, "INSERT INTO stxos VALUES(:outpoint, 0, 0, '', 1, 0, :transaction)");
	SQLite::Statement insert_txns(database, "INSERT INTO txns VALUES(:transaction, 0, 0, 0, 1, :raw)");
	// set :transaction to hash of :raw
	// set :outpoint to "${transaction}:broadcast"
	// execute in a transaction
	// reboot server
}
*/

vector<spvwallet::unspent> spvwallet::unspents(string scripthex)
{
	auto & database = this->database();
	SQLite::Statement query(database, "SELECT outpoint, value, scriptPubKey FROM utxos" + string(scripthex.size() ? " WHERE scriptPubKey = :publicKey" : ""));
	if (scripthex.size()) {
		query.bind(1, scripthex);
	}
	vector<unspent> result;
	while (query.executeStep()) {
		auto outpoint = query.getColumn(0).getString();
		auto parts = subprocess::util::split(outpoint, ":");
		result.emplace_back(unspent{
			parts[0],
			std::stoull(parts[1]),
			(uint64_t)query.getColumn(1).getInt64(),
			query.getColumn(2).getString()
		});
	}
	return result;
}

/* todo convert address to scriptPubKey portion
vector<string> spvwallet::transactions_received(string publicKey)
{
	auto & database = this->database();
	SQLite::Statement query(database, "SELECT outpoint FROM stxos  WHERE scriptPubKey = :publicKey UNION SELECT outpoint FROM utxos WHERE scriptPubKey = :publicKey");
	query.reset();
	query.bind(1, publicKey);
	vector<string> result;
	while (query.executeStep()) {
		auto outpoint = query.getColumn(0).getString();
		outpoint.resize(outpoint.find(':'));
		result.emplace_back(move(outpoint));
	}
	return result;
}

vector<string> spvwallet::transactions_sent(string publicKey)
{
	auto & database = this->database();
	SQLite::Statement query(database, "SELECT spendTxId FROM stxos WHERE scriptPubKey = :publicKey");
	query.reset();
	query.bind(1, publicKey);
	vector<string> result;
	while (query.executeStep()) {
		result.emplace_back(query.getColumn(0).getString());
	}
}
*/
