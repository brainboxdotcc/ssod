#include <dpp/dpp.h>
#include <ssod/combat.h>
#include <ssod/component_builder.h>
#include <ssod/game_util.h>

bool combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts) {
	return false;
}


void continue_combat(const dpp::interaction_create_t& event, player p) {
	dpp::cluster& bot = *(event.from->creator);
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_footer(dpp::embed_footer{ 
			.text = "In combat with " + p.combatant.name + ", Location " + std::to_string(p.paragraph),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994)
		.set_description("Fight! Fight! Fight!");
	p.save(event.command.usr.id);
	update_live_player(event, p);
	dpp::message m;
	m.add_embed(embed);
	component_builder cb(m);
	std::string label{"Dummy"}, id{"dummy"};
	dpp::component comp;
	comp.set_type(dpp::cot_button)
		.set_id(id)
		.set_label(label)
		.set_style(dpp::cos_danger)
		.set_emoji("👊", 0, false);
	cb.add_component(comp);
	cb.add_component(help_button());
	m = cb.get_message();

	event.reply(event.command.type == dpp::it_component_button ? dpp::ir_update_message : dpp::ir_channel_message_with_source, m.set_flags(dpp::m_ephemeral), [event, &bot, m, p](const auto& cc) {
		if (cc.is_error()) {{
			event.reply("Internal error displaying combat " + std::to_string(p.after_fragment) + " location " + std::to_string(p.paragraph) + ":\n```json\n" + cc.http_info.body + "\n```\nMessage:\n```json\n" + m.build_json() + "\n```");
		}}
	});

}