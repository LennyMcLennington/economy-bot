#include <dpp/commandhandler.h>
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <mongocxx/instance.hpp>

#include "config.hh"

int main() {
	// for some reason mongodb driver makes you do this at the start idk
	mongocxx::instance mongoinstance;

	dpp::cluster bot{econbot::config::token};
	dpp::commandhandler command_handler{&bot};

	dpp::command_handler x;

	command_handler.add_prefix("$");

	bot.on_ready([&](const dpp::ready_t &event) {
		fmt::print("Logged in as: {}#{:04d}\n", bot.me.username, bot.me.discriminator);
	});

	command_handler.add_command(
	    "75+", dpp::parameter_registration_t{dpp::parameter_type::pt_double},
	    [&](auto name, auto params, auto source) {

	    },
	    "GAMBLE MONEY");

	bot.start(false);
}
