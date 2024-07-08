/************************************************************************************
 *
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
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

#include <ssod/thread_pool.h>
#include <shared_mutex>

thread_pool::thread_pool(size_t num_threads) {
	for (size_t i = 0; i < num_threads; ++i) {
		threads.emplace_back([this, i]() {
			dpp::utility::set_thread_name("js/exec/" + std::to_string(i));
			while (true) {
				thread_pool_task task;
				{
					std::unique_lock<std::mutex> lock(queue_mutex);

					cv.wait(lock, [this] {
						return !tasks.empty() || stop;
					});

					if (stop && tasks.empty()) {
						return;
					}

					task = std::move(tasks.front());
					tasks.pop();
				}

				task();
			}
		});
	}
}

thread_pool::~thread_pool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}

	cv.notify_all();
	for (auto& thread : threads) {
		thread.join();
	}
}

void thread_pool::enqueue(thread_pool_task task)
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.emplace(std::move(task));
	}
	cv.notify_one();
}
