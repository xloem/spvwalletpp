#include <spvwallet.hpp>

#include "utils.hpp"

#include <thread>
#include <chrono>

using namespace std;

void spvwallet::waitForSync(std::function<void(uint64_t current_block, uint64_t total_blocks, uint64_t peers, std::string message)> status)
{
	size_t time_with_no_peers = 0;
	size_t time_with_no_progress = 0;
	size_t peerwipes = 0;

	uint64_t current_block = 0;
	uint64_t total_blocks = 0;
	vector<peer> peers;

	static constexpr auto FINDING_PEERS       = "Waiting for peers ...";
	static constexpr auto CONNECTING_STALLED  = "Stalled waiting for peers ...";
	static constexpr auto FINDING_TIP         = "Waiting for chain length ...";
	static constexpr auto DOWNLOADING         = "Downloading headers ...";
	static constexpr auto DOWNLOADING_STALLED = "Download stalled for up to 5s ...";
	static constexpr auto REBOOTING           = "Rebooting with new peers ...";
	static constexpr auto RESTARTING          = "Hard stall, recreating headers ...";
	static constexpr auto COMPLETE            = "Chain updated to tip.";

	string message;

	uint64_t delay = 400;

	while (true) {
		uint64_t downloaded = chaintip();
		if (downloaded != current_block) {
			current_block = downloaded;
			time_with_no_progress = 0;
			peerwipes = 0;
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
			message = time_with_no_peers ? CONNECTING_STALLED : FINDING_PEERS;
		} else {
			if (total_blocks == 0) {
				message = FINDING_TIP;
			} else if (total_blocks > current_block) {
				message = DOWNLOADING;
			} else {
				message = COMPLETE;
				if (status) { status(current_block, total_blocks, peers.size(), message); }
				return;
			}
			if (time_with_no_progress) {
				message = DOWNLOADING_STALLED;
			}
		}

		if (time_with_no_progress > 5000 && databaseaccess() && background) {
			if (peerwipes < 3 || !peers.size()) {
				if (peers.size()) {
					++ peerwipes;
				} else {
					peerwipes = 0;
				}
				message = REBOOTING;
				if (status) { status(current_block, total_blocks, peers.size(), message); }
			} else {
				message = RESTARTING;
				peerwipes = 0;
				if (status) { status(current_block, total_blocks, peers.size(), message); }
			}
			time_with_no_progress = 0;

			auto configuration = getconfiguration();
			configuration.mnemonic.clear();
			string repository_path = this->repository_path;
			stop();
			string oldpath = repository_path + PATH_SEPARATOR + "peers.json";
			string newpath = oldpath + ".stalled-" + to_string(time(0));
			rename(oldpath.c_str(), newpath.c_str());
			if (message == RESTARTING) {
				oldpath = repository_path + PATH_SEPARATOR + "headers.bin";
				newpath = oldpath + ".stalled-" + to_string(time(0));
				rename(oldpath.c_str(), newpath.c_str());
			}
			start(true, configuration);

			continue;
		}

		if (status) { status(current_block, total_blocks, peers.size(), message); }

		this_thread::sleep_for(chrono::milliseconds(delay));
	}
}

