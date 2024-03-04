/************************************************************************************
 *
 * Sporks, the learning, scriptable Discord bot!
 *
 * Copyright 2019 Craig Edwards <support@sporks.gg>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/

#pragma once
#include <cstdint>

/**
 * Command line variables
 */
struct commandline_config {
	/**
	 * Current cluster ID for this process
	 */
	uint32_t cluster_id{0};
	/**
	 * Total number of clusters that are running
	 */
	uint32_t max_clusters{0};
};

namespace commandline {
	/**
	 * Parse command line arguments, if passed, extracting the cluster id and max clusters arguments.
	 * Note that this uses POSIX getopt, and is NOT thread safe. However, once we have the arguments
	 * parsed into the return value, we never need call this again.
	 *
	 * @param argc argument count from main()
	 * @param argv argument vector from main()
	 * @return configuration parsed from command line
	 */
	commandline_config parse(int argc, char const *argv[]);
}
