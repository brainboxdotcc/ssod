#pragma once

#include <string>
#include <functional>
#include <dpp/dpp.h>

/**
 * @brief The results of a censor filter API call
 */
struct swear_filter_t {
        /**
         * @brief True if the text is clean of swearing
         */
        bool clean = true;
        /**
         * @brief The censored content with swearwords replaced by # symbols
         */
        std::string censored_content;
};

/**
 * @brief Censor API callback function
 */
typedef std::function<void(const swear_filter_t&)> swear_filter_event_t;

/**
 * @brief A class for calling Neutrino API censor endpoint
 * The censor endpoint filters swearwords from text in multiple languages.
 * It is maintained by a third party and is much better than any simple
 * list i could create and maintain myself.
 */
class neutrino
{
        /**
         * @brief D++ cluster
         */
        dpp::cluster* cluster;

        /**
         * @brief API user ID
         */
        std::string user_id;

        /**
         * @brief API key
         */
        std::string api_key;
public:
        /**
         * @brief Construct a new neutrino object
         *
         * @param cl D++ Cluster
         * @param userid API user ID
         * @param apikey API Key
         */
        neutrino(dpp::cluster* cl, const std::string& userid, const std::string& apikey);

        /**
         * @brief Returns a swear_filter_t in a callback after checking text for swearing.
         * Uses a HTTPS REST API call.
         *
         * @param text text to check
         * @param callback callback to receive swear filter result
         */
        void contains_bad_word(const std::string& text, swear_filter_event_t callback);
};
