#pragma once

#include <stdexcept>
#include <string>

class spvwallet
{
public:
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

	spvwallet(std::string path = "spvwallet", bool startInBackgroundIfNotRunning = true);

	std::string version();
	void start(bool background);

	std::string currentaddress();
	double balance();

	bool isRunning();

private:
	std::string prefix;

	std::string command(std::string commands, bool output = false, bool wait = true, void ** stream_pointer = 0);
};
