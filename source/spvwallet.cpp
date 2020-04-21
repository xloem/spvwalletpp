#include <spvwallet.hpp>


using namespace std;

#include <string>
#include <iomanip>
#include <iostream>
string process(string command, bool output = false, bool wait = true, FILE ** stream_pointer = 0)
{
	string result;
	char buffer[1024];
	FILE *stream = popen(command.c_str(), "r");
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

string spvwallet::command(string commands, bool output, bool wait, void ** stream_pointer)
{
	string result = process(prefix + commands + " 2>&1", output, wait, (FILE**)stream_pointer);

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
void spvwallet::start(bool background)
{
	if (background) {
		command("start", false, false); 
	} else {
		command("start", true);
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

spvwallet::spvwallet(std::string path, bool startInBackgroundIfNotRunning)
: prefix(path + " ")
{
	if (startInBackgroundIfNotRunning) {
		try {
			currentaddress();
		} catch (error::unavailable &) {
			start(true);
		}
	}
}

string spvwallet::version()
{
	return command("version");
}

string spvwallet::currentaddress()
{
	return command("currentaddress");
}
