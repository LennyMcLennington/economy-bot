#include <dpp/commandhandler.h>
#include <dpp/dpp.h>
#include <fmt/format.h>

#include "config.hh"

int main() {
	dpp::cluster bot{econbot::config::token};
	dpp::commandhandler command_handler{&bot};

	command_handler.add_prefix("$");

	bot.on_ready([&](const dpp::ready_t &event) {
		fmt::print("Logged in as: {}#{:04d}\n", bot.me.username, bot.me.discriminator);
	});

	bot.start(false);
}
