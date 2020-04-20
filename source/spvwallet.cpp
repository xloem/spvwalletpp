#include <spvwallet.hpp>


using namespace std;

#include <string>
#include <iomanip>
string execute(string cmd)
{
	string result;
	char buffer[1024];
	FILE *stream = popen(cmd.c_str(), "r");
	while (fgets(buffer, sizeof(buffer), stream) != 0) {
		result.append(buffer);
	}
	pclose(stream);
	return result;
}

string spvwallet(string command)
{
	string result = execute("spvwallet " + command);
	if (result.compare(0, 10, "rpc error:") == 0) {
		throw spvwallet::error::make(result.substring(10));
	}
	return result; // suspect result may be json or line-delimited
}

spvwallet::error::error(string code, string description)
: runtime_error(code + ": " + description)
{ }

spvwallet::error spvwallet::error::make(string description)
{
	description = " " + description;
	auto codestart = description.find(" code = ");
	auto descstart = description.find(" desc = ");
	auto code = result.substr(codestart + 8, descstart);
	auto desc = result.substr(descstart + 8);
	if (code == "Unavailable") {
		return unavailable(code, description);
	} else {
		return error(code, description);
	}
}
