#include <patron++>
int main()
{
	using namespace patron::cmds;
	namespace r = patron::cmds::readers;
	CommandHandler<> handler;

	handler.register_command({"cmd", "cmd_alias"},
	                         std::tuple{Argument<r::Double>{"the number"}, Argument<r::String>{"the string"}},
	                         [](double arg1, std::string arg2) {
		                         std::cout << arg1 << "\n";
		                         std::cout << arg2 << "\n";
	                         });

	handler.handle_command(R"(cmd "1.5" "this is using the main cmd name")");
	handler.handle_command(R"(cmd_alias "1.7" "this is using the alias")");
}
