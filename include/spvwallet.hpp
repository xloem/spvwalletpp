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
			MAIN,
			TEST,
			REGRESSION
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
		enum {
			UNCONFIRMED,
			PENDING,
			CONFIRMED,
			STUCK,
			DEAD,
			ERROR
		} status;
		uint64_t timestamp;
		uint64_t confirmations;
		uint64_t height;
		bool watchOnly;
	};
	struct peer
	{
		std::string address;
		uint64_t bytesSent;
		uint64_t bytesReceived;
		bool connected;
		uint64_t id;
		uint64_t lastBlock;
		uint64_t protocolVersion;
		std::string services;
		std::string userAgent;
		uint64_t timeConnected;
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
	~spvwallet();

	std::string version();
	void start(bool background, configuration _configuration);
	void stop();

	uint64_t chaintip();
	std::string currentaddress();
	uint64_t balance();
	std::vector<transaction> transactions();
	std::vector<peer> peers();

	void resyncblockchain(uint64_t timestamp);

	bool running();

private:
	std::string prefix;
	uint64_t pid;

	std::string command(std::vector<std::string> commands, bool output = false, std::string return_at_output = {}, uint64_t * pid_pointer = 0);
};
