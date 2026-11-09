#include <dpp/cluster.h>
#include <dpp/queues.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>
#include <ssod/standalone.h>
#include <charconv>
#include <string_view>
#include <ssod/game_player.h>
#include <ssod/ssod.h>
#include <mutex>

using post_request_fn = void (*)(dpp::request_concurrency_queue*, std::unique_ptr<dpp::http_request>);

extern "C" void ssod_post_request(dpp::request_concurrency_queue* self, std::unique_ptr<dpp::http_request> req) __asm__("_ZN3dpp25request_concurrency_queue12post_requestESt10unique_ptrINS_12http_requestESt14default_deleteIS2_EE");

static uint32_t standalone_user_id(const dpp::http_request& req) {
	const std::string_view endpoint = req.endpoint;
	const std::string_view parameters = req.parameters;

	if (!endpoint.starts_with("/api/v10/interactions/") && !endpoint.starts_with("/api/v10/webhooks/")) {
		return 0;
	}

	if (!parameters.starts_with("ssod_")) {
		return 0;
	}

	const auto separator = parameters.find('/');
	if (separator == std::string_view::npos) {
		return 0;
	}

	const auto route = parameters.substr(separator);
	if (route != "/callback" && !route.starts_with("/messages/")) {
		return 0;
	}

	const auto id = parameters.substr(5, separator - 5);
	uint32_t user_id = 0;

	const auto [ptr, ec] = std::from_chars(id.data(), id.data() + id.size(), user_id);
	if (ec != std::errc{} || ptr != id.data() + id.size()) {
		return 0;
	}

	return user_id;
}

extern "C" void ssod_post_request(dpp::request_concurrency_queue* self, std::unique_ptr<dpp::http_request> req) {
	static auto original = reinterpret_cast<post_request_fn>(dlsym(RTLD_NEXT, "_ZN3dpp25request_concurrency_queue12post_requestESt10unique_ptrINS_12http_requestESt14default_deleteIS2_EE"));
	if (!original) {
		std::fprintf(stderr, "SSOD REST hook: unable to resolve original: %s\n", dlerror());
		std::abort();
	}
	if (req->parameters.starts_with("ssod_")) {
		const uint32_t user_id = standalone_user_id(*req);

		dpp::http_request_completion_t result;

		if (!user_id) {
			std::fprintf(stderr, "SSOD REST hook: invalid standalone interaction route\n");
			result.status = 400;
			req->complete(result);
			return;
		}

		try {
			auto response = dpp::json::parse(req->postdata);

			response["ssod_user_id"] = user_id;
			if (auto username = get_cached_player_name(user_id)) {
				response["ssod_username"] = *username;
			}

			ssod::standalone::output(user_id, response);
			result.status = 204;
		} catch (const std::exception& e) {
			self->creator->log(dpp::ll_error, "SSOD REST hook: " + std::string(e.what()));
			result.status = 500;
			result.body = R"({"message":"Standalone response failed"})";
		}

		req->complete(result);
		return;
	}

	original(self, std::move(req));
}
