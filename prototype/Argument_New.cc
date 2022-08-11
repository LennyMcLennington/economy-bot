#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

namespace command_handler
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
} // namespace command_handler

std::string StringReader(const std::string &s)
{
	return s;
}

double DoubleReader(const std::string &s)
{
	return std::stod(s);
}

class ArgumentInterface
{
public:
	virtual const std::string &name() = 0;
	virtual ~ArgumentInterface() = default;
};

template <typename T> class Argument : public ArgumentInterface
{
public:
	using result_type = T;
	using reader_type = std::function<result_type(std::string)>;
	reader_type m_reader;
	std::string m_name;
	const std::string &name()
	{
		return m_name;
	}
	Argument(reader_type parse, std::string name) : m_reader{parse}, m_name{name}
	{
	}
};

template <typename T> Argument(T &&, std::string) -> Argument<std::invoke_result_t<T, const std::string &>>;

struct CommandInterface
{
public:
	// virtual const std::vector<std::string> &aliases() = 0;
	virtual void run(std::string args) = 0;
	virtual ~CommandInterface() = default;
};

template <typename... T> class CommandHandler
{
	std::unordered_map<std::string, CommandInterface&> x;
	void handle_command(T... context, std::string command)
	{
	}
};

template <typename... T> struct Command;

template <typename... ArgTypes, typename... ExtraArgTypes>
struct Command<std::tuple<ArgTypes...>, std::tuple<ExtraArgTypes...>> : public CommandInterface
{
	using exec_func_typye = std::function<void(ExtraArgTypes..., ArgTypes...)>;
	using arg_tuple_type = std::tuple<ArgTypes...>;
	using arginfo_tuple_type = std::tuple<Argument<ArgTypes>...>;

	arginfo_tuple_type m_arginfo;
	exec_func_typye m_exec_func;

	template <std::size_t... Indexes>
	arg_tuple_type parse_args(std::string &args, std::integer_sequence<std::size_t, Indexes...>)
	{
		return arg_tuple_type{std::get<Indexes>(m_arginfo).m_reader(command_handler::parse_single_argument(args))...};
	}

	const std::vector<std::reference_wrapper<ArgumentInterface>> &args()
	{
		const static std::vector<std::reference_wrapper<ArgumentInterface>> args_vec(std::apply(
		    [](auto &&...args) {
			    return std::vector<std::reference_wrapper<ArgumentInterface>>{
			        dynamic_cast<ArgumentInterface &>(args)...};
		    },
		    m_arginfo));
		return args_vec;
	}

	void run(ExtraArgTypes... extra_args, std::string content)
	{
		arg_tuple_type res = parse_args(content, std::make_index_sequence<std::tuple_size_v<decltype(m_arginfo)>>{});
		if (!command_handler::parse_single_argument(content).empty())
		{
			// FIXME: needs an exception or something for the situation where too many or few arguments were passed
		}
		std::apply(m_exec_func, std::tuple_cat(std::make_tuple(extra_args...), res));
	}

	Command(CommandHandler<ExtraArgTypes...>, std::tuple<Argument<ArgTypes>...> args, exec_func_typye f) : m_arginfo{args}, m_exec_func{f}
	{
	}
};

template <typename T, typename... ArgTypes, typename... ExtraArgTypes>
Command(CommandHandler<ExtraArgTypes...>, std::tuple<Argument<ArgTypes>...>, T &&)
    -> Command<std::tuple<ArgTypes...>, std::tuple<ExtraArgTypes...>>;

int main()
{
	CommandHandler<> handler;
	Command x{handler, std::make_tuple(Argument{StringReader, "Arg1Name"}, Argument{StringReader, "Arg2Name"}),
	          [](std::string arg1, std::string arg2) {
		          std::cout << arg1 << "\n";
		          std::cout << arg2 << "\n";
	          }};
	x.run("\"1.5\" \"hello world\"");
}
