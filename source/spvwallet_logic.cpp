#include <spvwallet.hpp>

#include "utils.hpp"

#include <thread>
#include <chrono>

using namespace std;

void spvwallet::waitForSync(std::function<void(uint64_t current_block, uint64_t total_blocks, uint64_t peers, std::string message)> status)
{
	size_t time_with_no_peers = 0;
	size_t time_with_no_progress = 0;

	uint64_t current_block = 0;
	uint64_t total_blocks = 0;
	vector<peer> peers;

	static constexpr auto FINDING_PEERS = "Waiting for peers ...";
	static constexpr auto FINDING_TIP = "Waiting for chain length ...";
	static constexpr auto DOWNLOADING = "Downloading blocks ...";
	static constexpr auto REBOOTING = "Stalled, rebooting with new peers ...";

	string message;

	uint64_t delay = 250;

	while (true) {
		uint64_t downloaded = chaintip();
		if (downloaded != current_block) {
			current_block = downloaded;
			time_with_no_progress = 0;
		} else {
			time_with_no_progress += delay;
		}

		peers = this->peers();
		for (auto & peer : peers) {
			if (peer.lastBlock > total_blocks) {
				total_blocks = peer.lastBlock;
			}
		}

		if (peers.size() == 0) {
			time_with_no_peers += delay;
			message = FINDING_PEERS;
			if (time_with_no_peers) {
				message	+= " (stalled for " + to_string(time_with_no_peers) + " ms)";
			}
		} else {
			if (total_blocks == 0) {
				message = FINDING_TIP;
			} else if (total_blocks > current_block) {
				message = DOWNLOADING;
			} else {
				break;
			}
			if (time_with_no_progress) {
				message	+= " (stalled for " + to_string(time_with_no_progress) + " ms)";
			}
		}

		if (time_with_no_progress > 5000 && databaseaccess() && background) {
			message = REBOOTING;
			status(current_block, total_blocks, peers.size(), message);

			auto configuration = getconfiguration();
			configuration.mnemonic.clear();
			string repository_path = this->repository_path;
			stop();
			string oldpath = repository_path + PATH_SEPARATOR + "peers.json";
			string newpath = oldpath + ".stalled-" + to_string(time(0));
			rename(oldpath.c_str(), newpath.c_str());
			start(true, configuration);
			continue;
		}

		status(current_block, total_blocks, peers.size(), message);

		this_thread::sleep_for(chrono::milliseconds(delay));
	}
}

