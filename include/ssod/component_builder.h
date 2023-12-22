#pragma once
#include <dpp/dpp.h>
#include <string>
#include <set>

/**
 * @brief Allows components to be added continually to a message, without worrying about component
 * rows, maximum number of buttons per row, maximum number of rows, or duplicate custom ids.
 */
struct component_builder {
	/**
	 * @brief Current component index, when this % 5 == 0, a new row is inserted
	 */
	size_t index{0};
	/**
	 * @brief Parent component row count
	 */
	size_t component_parent{0};
	/**
	 * @brief A set of existing IDs used to ignore duplicates
	 */
	std::set<std::string> ids;
	/**
	 * @brief Message to operate on
	 */
	dpp::message message;
	/**
	 * @brief Constructor
	 * @param m Message
	 */
	component_builder(const dpp::message& m);
	/**
	 * @brief Add a component to the message.
	 * If there are too many components or this one already exists,
	 * the component will be silently dropped.
	 * 
	 * @param c Component
	 */
	void add_component(const dpp::component& c);
	/**
	 * @brief Get the completed message
	 *
	 * @return dpp::message& reference to completed message
	 */
	const dpp::message& get_message();
};

