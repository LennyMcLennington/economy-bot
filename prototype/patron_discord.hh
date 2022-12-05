#ifndef PATRON_DISCORD_HH
#define PATRON_DISCORD_HH
#include "patron_cmds.hh"
namespace patron::discord
{
struct CommandContext;
using BaseCommandHandler = patron::cmds::CommandHandler<CommandContext>;

class DiscordCommandHandler : BaseCommandHandler
{
};

struct CommandContext
{
	DiscordCommandHandler &handler;

};
}

#endif // PATRON_DISCORD_HH
