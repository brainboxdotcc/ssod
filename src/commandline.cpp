#include <ssod/commandline.h>
#include <getopt.h>
#include <unistd.h>
#include <ssod/ssod.h>
#include <iostream>
#include <cstdlib>

namespace commandline {
	commandline_config parse(int argc, char const *argv[]) {

		struct option long_opts[] = {
			{ "clusterid", required_argument, nullptr, 'c' },
			{ "maxclusters", required_argument, nullptr, 'm' },
			{ "showcommands", optional_argument, nullptr, 's' },
			{ nullptr, 0, nullptr, 0 }
		};

		int index{0};
		int arg;
		bool clusters_defined{false}, show_commands{false};
		uint32_t cluster_id{0};
		uint32_t max_clusters{1};

		/**
		 * BIG FAT WARNING: https://nullprogram.com/blog/2014/10/12/
		 * getopt_long_only maybe not thread safe. This is only called before the bot is initialised, so
		 * it's generally "ok-ish".
		 */
		opterr = 0;
		while ((arg = getopt_long_only(argc, (char* const*)argv, "", long_opts, &index)) != -1) {
			switch (arg) {
				case 0:
					break;
				case 'c':
					/* Cluster id */
					cluster_id = atoi(optarg);
					clusters_defined = true;
					break;
				case 'm':
					/* Number of clusters */
					max_clusters = atoi(optarg);
					break;
				case 's':
					/* Number of clusters */
					show_commands = true;
					break;
				case '?':
				default:
					std::cerr << "Unknown parameter '" << argv[optind - 1] << "'\n";
					std::cerr << "Usage: " << argv[0] << " [-clusterid <n>] [-maxclusters <n>]\n\n";
					std::cerr << "-clusterid <n>:    The current cluster id to identify for, must be set with -maxclusters\n";
					std::cerr << "-maxclusters <n>:  The maximum number of clusters the bot is running, must be set with -clusterid\n";
					std::cerr << "-showcommands:     Output JSON definitions of all application commands\n";
					exit(1);
			}
		}

		if (clusters_defined && max_clusters == 0) {
			std::cerr << "ERROR: You have defined a cluster id with -clusterid but no cluster count with -maxclusters.\n";
			exit(2);
		}

		return commandline_config{.cluster_id =  cluster_id, .max_clusters = max_clusters, .display_commands = show_commands};
	}
}
