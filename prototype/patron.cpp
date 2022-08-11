import <functional>;
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

namespace patron::parser
{
enum class quotation
{
	not_quoted,
	double_quoted,
	single_quoted
};

std::string parse_single_argument(std::string &content)
{
	std::string output;

	bool whitespace_state = false;
	quotation quote_state = quotation::not_quoted;
	bool escape_state = false;

	auto it = content.begin();
	// skip leading whitespace
	for (; it != content.end(); ++it)
	{
		if (!isspace(*it))
			break;
	}

	for (; it != content.end(); ++it)
	{
		if (std::isspace(*it) && quote_state == quotation::not_quoted && escape_state == false)
		{
			whitespace_state = true;
		}
		else
		{
			if (!escape_state && *it == '\\')
			{
				escape_state = true;
			}
			else
			{
				if (whitespace_state)
				{
					content.erase(content.begin(), it);
					whitespace_state = false;
					return output;
				}
				if (escape_state)
				{
					// Default is to ignore escaping by just outputting the backslash
					// back where it would've been in the original input.
					switch (*it)
					{
					default:
						output.push_back('\\');
					case '\\':
					case '"':
					case '\'':
					case ' ':
						output.push_back(*it);
					}
					escape_state = false;
				}
				else if (quote_state != quotation::single_quoted && *it == '"')
				{
					quote_state =
					    quote_state == quotation::not_quoted ? quotation::double_quoted : quotation::not_quoted;
				}
				else if (quote_state != quotation::double_quoted && *it == '\'')
				{
					quote_state =
					    quote_state == quotation::not_quoted ? quotation::single_quoted : quotation::not_quoted;
				}
				else
				{
					output.push_back(*it);
				}
			}
		}
	}

	return output;
}
} // namespace patron::parser

namespace patron::cmds
{
namespace readers
{
struct String
{
	constexpr std::string operator()(const std::string &s)
	{
		return s;
	}
};

struct Double
{
	double operator()(const std::string &s)
	{
		return std::stod(s);
	}
};
} // namespace readers

class ArgumentInterface
{
public:
	virtual ~ArgumentInterface() = default;
	virtual const std::string &desc() const = 0;
};

template <typename Reader> class Argument : public ArgumentInterface
{
public:
	using reader = Reader;
	using result_type = std::invoke_result_t<decltype(&reader::operator()), reader, const std::string &>;
	Argument(std::string description) : m_desc{description}
	{
	}
	const std::string &desc() const override
	{
		return m_desc;
	}

private:
	std::string m_desc;
};

template <typename T> Argument(T &&, std::string) -> Argument<std::invoke_result_t<T, const std::string &>>;

template <typename... CommandContextTypes> class CommandHandler
{
public:
	struct CommandInterface;
	std::unordered_map<std::string, std::shared_ptr<CommandInterface>> m_commands;
	void handle_command(CommandContextTypes... context, std::string command) const
	{
		std::string command_name = patron::parser::parse_single_argument(command);
		std::cout << "Running command... \"" << command_name << "\"\n";
		if (m_commands.contains(command_name))
		{
			m_commands.at(command_name)->run(context..., command);
		}
	}

	void register_command(std::shared_ptr<CommandInterface> cmd)
	{
		for (std::string name : cmd->aliases())
		{
			std::cout << "Adding alias: " << name << "\n";
			if (!m_commands.contains(name))
			{
				m_commands[name] = cmd;
			}
		}
	}
	void register_command(auto... args)
	{
		register_command(std::shared_ptr<CommandInterface>{new Command{args...}});
	}
	void register_command(std::initializer_list<std::string> &&a1, auto... args)
	{
		register_command(std::shared_ptr<CommandInterface>{new Command{a1, args...}});
	}

	struct CommandInterface
	{
	public:
		virtual void run(std::string args) const = 0;
		virtual std::span<const std::string> aliases() const = 0;
		virtual ~CommandInterface() = default;
	};

	template <typename... T> struct Command;

	template <typename... ArgReaders> struct Command<std::tuple<ArgReaders...>> : public CommandInterface
	{
		std::vector<std::string> m_aliases;
		std::span<const std::string, std::dynamic_extent> aliases() const override
		{
			return m_aliases;
		}

		using exec_func_type =
		    std::function<void(CommandContextTypes..., typename Argument<ArgReaders>::result_type...)>;
		using arg_tuple_type = std::tuple<typename Argument<ArgReaders>::result_type...>;
		using arginfo_tuple_type = std::tuple<Argument<ArgReaders>...>;

		arginfo_tuple_type m_arginfo;
		exec_func_type m_exec_func;

		template <std::size_t... Indexes>
		arg_tuple_type parse_args(std::string &args, std::integer_sequence<std::size_t, Indexes...>) const
		{
			return arg_tuple_type{typename std::remove_reference_t<decltype(std::get<Indexes>(m_arginfo))>::reader{}(
			    patron::parser::parse_single_argument(args))...};
		}

		const std::vector<std::reference_wrapper<ArgumentInterface>> args_vec{std::apply(
		    [](auto &&...args) { return std::vector<std::reference_wrapper<ArgumentInterface>>{args...}; }, m_arginfo)};
		const std::vector<std::reference_wrapper<ArgumentInterface>> &args()
		{
			return args_vec;
		}

		void run(CommandContextTypes... extra_args, std::string content) const override
		{
			arg_tuple_type res =
			    parse_args(content, std::make_index_sequence<std::tuple_size_v<decltype(m_arginfo)>>{});
			if (!patron::parser::parse_single_argument(content).empty())
			{
				// TODO: handle the situation where too many or few arguments were passed
			}
			std::apply(m_exec_func, std::tuple_cat(std::make_tuple(extra_args...), res));
		}

		Command(std::string name, arginfo_tuple_type args, exec_func_type f) : Command({name}, args, f)
		{
		}
		Command(std::initializer_list<std::string> aliases, arginfo_tuple_type args, exec_func_type f)
		    : m_aliases{aliases}, m_arginfo{args}, m_exec_func{f}
		{
		}
	};

	template <typename... ArgTypes>
	Command(auto &&, std::tuple<Argument<ArgTypes>...>, auto &&) -> Command<std::tuple<ArgTypes...>>;

	template <typename... ArgTypes>
	Command(std::initializer_list<std::string>, std::tuple<Argument<ArgTypes>...>, auto &&)
	    -> Command<std::tuple<ArgTypes...>>;
};
} // namespace patron::cmds

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
