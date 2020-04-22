#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
	#define PATH_SEPARATOR '\\'
#else
	#define PATH_SEPARATOR '/'
#endif

#include <time.h>
static uint64_t from_iso8601(std::string datetime)
{
	struct tm tm;
	strptime(datetime.c_str(), "%FT%T%z", &tm);
	return mktime(&tm);
}
static std::string to_iso8601(uint64_t timestamp)
{
	time_t timestamp_proper = timestamp;
	struct tm * tm_p = gmtime(&timestamp_proper);
	static thread_local char buffer[1024];
	strftime(buffer, sizeof(buffer), "%FT%TZ", tm_p);
	return buffer;
}

#include <subprocess.hpp>
#include <iostream>
static std::string process(std::vector<std::string> const & commands, bool output = false, std::string return_at_output = {}, std::unique_ptr<subprocess::Popen> * pointer = 0)
{
	std::unique_ptr<subprocess::Popen> process(new subprocess::Popen(commands, output ? subprocess::output{stdout} : subprocess::output{subprocess::PIPE}, output ? subprocess::error{stderr} : subprocess::error{subprocess::PIPE}));
	std::string result;
	if (return_at_output.size() == 0) {
		process->wait();
		auto results = process->communicate();
		result = std::string(results.second.buf.data(), results.second.length) + std::string(results.first.buf.data(), results.first.length);
	} else {
		size_t idx = 0;
		char buffer[256] = { 0 };
		while (result.find(return_at_output, idx) == std::string::npos) {
			idx = result.size() - return_at_output.size() + 1;
			if (0 == fgets(buffer, sizeof(buffer), process->output())) {
				// throw error maybe?
				break;
			}
			
			result.append(buffer);
			if (output) {
				std::cout << buffer;
			}
		}
		if (output) {
			std::cout << std::endl;
		}
	}
	if (pointer) *pointer = move(process);
	return result;
}


#include <SQLiteCpp/SQLiteCpp.h>

struct spvwallet::database_struct
{
	SQLite::Database database;
};
