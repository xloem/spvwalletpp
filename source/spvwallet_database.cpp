#include <spvwallet.hpp>

#include "utils.hpp"

#include <SQLiteCpp/SQLiteCpp.h>

using namespace std;

SQLite::Database & spvwallet::database()
{
	if (!_database) {
		_database.reset(new spvwallet::database_struct{{repository_path + PATH_SEPARATOR + "wallet.db"}});
	}
	return _database->database;
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
