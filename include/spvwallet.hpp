#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace subprocess { class Popen; }

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
		uint64_t walletCreationDate; // to speed syncing, earliest transaction
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

		std::string const code;
		std::string const description;

		class unavailable;
		class internal;
	};
	class error::unavailable : public error { public: using error::error; };
	class error::internal : public error { public: using error::error; };

	spvwallet(std::string path = "spvwallet", bool startInBackgroundIfNotRunning = true, configuration startConfiguration = {});
	~spvwallet();

	std::string version();
	void start(bool background, configuration _configuration);
	void stop();

	uint64_t chaintip();
	std::string currentaddress();
	uint64_t balance();
	void addwatchedaddress(std::string address);
	std::vector<std::string> listaddresses();
	std::vector<transaction> transactions();
	std::vector<peer> peers();

	void resyncblockchain(uint64_t timestamp);

	bool running();

private:
	std::string prefix;
	std::unique_ptr<subprocess::Popen> background;

	std::string command(std::vector<std::string> commands, bool output = false, std::string return_at_output = {}, std::unique_ptr<subprocess::Popen> * pointer = 0);
};
