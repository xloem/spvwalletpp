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
		class unavailable : public error { public: using error::error; };
		static error make(std::string description);
	};
	static bool isRunning();
	static void start();
	static void stop();
};
