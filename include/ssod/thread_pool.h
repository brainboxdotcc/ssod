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
#pragma once
#include <dpp/dpp.h>
#include <queue>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <functional>

/**
 * A task within a thread pool. A simple lambda that accepts no parameters and returns void.
 */
using thread_pool_task = std::function<void()>;

/**
 * @brief A thread pool contains 1 or more worker threads which accept thread_pool_task lambadas
 * into a queue, which is processed in-order by whichever thread is free.
 */
struct thread_pool {
	std::vector<std::thread> threads;
	std::queue<thread_pool_task> tasks;
	std::mutex queue_mutex;
	std::condition_variable cv;
	bool stop{false};

	explicit thread_pool(size_t num_threads = std::thread::hardware_concurrency());
	~thread_pool();
	void enqueue(thread_pool_task task);
};
