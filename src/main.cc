#include "../prototype/patron.hh"
#include <dpp/commandhandler.h>
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>

#include "config.hh"


int main() {
	// for some reason mongodb driver makes you do this at the start idk
	mongocxx::instance mongoinstance;
	mongocxx::client client{};

	dpp::cluster bot{econbot::config::token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members};

	bot.on_ready([&](const dpp::ready_t &event) {
		fmt::print("Logged in as: {}#{:04d}\n", bot.me.username, bot.me.discriminator);
	});

	using namespace patron::cmds;
	namespace r = patron::cmds::readers;
	struct CommandContext;
	using Handler = CommandHandler<CommandContext>;

	struct CommandContext
	{
		Handler &handler;
		mongocxx::client &db;
		dpp::message_create_t event;
	};

	Handler handler;

	handler.register_command<0>({"cmd", "cmd_alias"}, "The description of the command", "general",
							 Handler::CommandOptions{},
							 std::tuple{Argument<r::Double>{"the number"}, Argument<r::String>{"the string"}},
							 [](CommandContext context, double arg1, std::string arg2) {
								 context.event.reply("TEST.");
								 std::cout << arg1 << "\n";
								 std::cout << arg2 << "\n";
							 });

	bot.on_message_create([&](const dpp::message_create_t &event) {
		std::cout << "Message: " << event.msg.content << "by: " << event.msg.author.username << "\n";
		handler.handle_command({handler, client, event}, event.msg.content);
	});
	bot.start(false);
}
