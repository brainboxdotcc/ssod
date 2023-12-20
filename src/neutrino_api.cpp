#include <string>
#include <cstdint>
#include <ssod/neutrino_api.h>

neutrino::neutrino(dpp::cluster* cl, const std::string& userid, const std::string& apikey) : cluster(cl), user_id(userid), api_key(apikey) {
}

void neutrino::contains_bad_word(const std::string& text, swear_filter_event_t callback) {
        std::string request = nlohmann::json({
                { "content", text },
                { "censor-character", "#" },
                { "catalog", "strict" },
                { "output-format", "JSON" },
        }).dump();
        this->cluster->request("https://neutrinoapi.net/bad-word-filter", dpp::m_post, [this, text, callback](const auto& rv) {
                        swear_filter_t sf;
                        nlohmann::json j;
			sf.clean = true;
			sf.censored_content = text;
                        if (rv.error == dpp::h_success && !rv.body.empty()) {
                                try {
                                        j = nlohmann::json::parse(rv.body);
                                        if (j.contains("is-bad") && j.at("is-bad").get<bool>()) {
                                                sf.clean = false;
                                                sf.censored_content = j.at("censored-content").get<std::string>();
                                        }
                                }
                                catch (const std::exception &e) {
                                        callback(sf);
                                }
                        }
                        callback(sf);
                },
                request,
                "application/json",
                {
                        { "Content-Type", "application/json" },
                        { "Accept", "application/json" },
                        { "user-id", this->user_id },
                        { "api-key", this->api_key },
                }
        );
}
