#pragma once

#include <stdexcept>
#include <string>
#include <vector>

class spvwallet
{
public:
	struct configuration
	{
		configuration();
		std::string dataDirectory;
		enum
		{
			BITCOIN,
			TESTNET,
			REGTEST
		} network;
		std::string mnemonic; // may be used to recreate a wallet
		uint64_t mnemonicDate; // to speed syncing, earliest transaction
		std::string trustedPeer;
		bool tor;
	};
	struct transaction
	{
		std::string txid;
		uint64_t value;
		uint64_t timestamp;
		uint64_t confirmations;
		uint64_t height;
		bool watchOnly;
	};

	class error : public std::runtime_error
	{
	public:
		error(std::string code, std::string description);
		static void makeAndThrow(std::string description);

		class unavailable;
	};
	class error::unavailable : public error
	{
	public:
		using error::error;
	};

	spvwallet(std::string path = "spvwallet", bool startInBackgroundIfNotRunning = true, configuration startConfiguration = {});

	std::string version();
	void start(bool background, configuration _configuration);

	std::string currentaddress();
	uint64_t balance();
	std::vector<transaction> transactions();

	bool isRunning();

private:
	std::string prefix;

	std::string command(std::vector<std::string> commands, bool output = false, bool wait = true, void ** stream_pointer = 0);
};
