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
		static error make(std::string description);

		class unavailable;
	};
	class error::unavailable : public error
	{
	public:
		using error::error;
	};
	static bool isRunning();
	static void start();
	static void stop();
};
