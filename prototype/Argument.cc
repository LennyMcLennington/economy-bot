#include <any>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>

#include "../subprojects/commandhandler/include/command_handler/command.hh"
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

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
	std::cout << "CONTENT: " << content << "\n";
	std::string output;

	// Starting in whitespace state ensures that emplace_back() will be called
	// as soon as the first non-whitespace character is found, since when we are
	// in non-whitespace mode it's assumed that the vector has had the
	// emplace_back() already called.
	bool whitespace_state = true;
	bool first_whitespace_state_done = false;

	quotation quote_state = quotation::not_quoted;
	bool escape_state = false;

	for (auto it = content.begin(); it != content.end(); ++it)
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
					if (first_whitespace_state_done) {
					content.erase(content.begin(), it);
					whitespace_state = false;
					std::cout << "Output: " << output << "\n";
					return output;
					}
					else {
						first_whitespace_state_done = true;
						whitespace_state = false;
					}
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
					std::cout << "um" << "\n";
					output.push_back(*it);
				}
			}
		}
	}

	return output;
}
} // namespace command_handler

class ArgType
{
  public:
	virtual ~ArgType() = 0;
	virtual std::any parse_arg(std::string_view s) = 0;
};

class StringType : public ArgType
{
  public:
	std::any parse_arg(std::string_view s)
	{
		return std::string{s};
	}
};

/* class Argument
{
  private:
    struct Options
    {
        bool optional = false;
    };
    struct Options options;

    std::string key;
    std::string type;

  public:
    Argument(std::string key, std::string type) : key{key}, type{type}
    {
    }
}; */

/* class Command
{
  private:
    std::string x;
    std::initializer_list<Argument> y;

  public:
    Command(std::initializer_list<Argument> y) : y{y}
    {
    }
}; */

class CommandGeneric
{
  public:
	virtual void run(std::string args) = 0;
	virtual ~CommandGeneric() = default;
};

class StringArgument
{
  public:
	static std::string parse(const std::string &s)
	{
		return s;
	}
};

class DoubleArgument
{
	public:
		static double parse(const std::string &s)
		{
			return std::stod(s);
		}
};

template <typename T> class Argument
{
  public:
	using type = T;
	std::string name;
	Argument(std::string s) : name{s}
	{
	}
};

template <typename... Arguments> class Command : public CommandGeneric
{
  public:
	std::tuple<Arguments...> args;
	using thing = std::tuple<typename std::invoke_result_t<decltype(&Arguments::type::parse), std::string &>...>;

	template <std::size_t index, typename T, typename... U> void parse_args(std::string &args, thing &out)
	{
		std::get<index>(out) = T::type::parse(command_handler::parse_single_argument(args));
		if constexpr (sizeof...(U) > 0)
		{
			parse_args<index - 1, U...>(args, out);
		}
	}

	void exec(thing args)
	{
		std::cout << "hi";
		std::cout << std::get<0>(args);
	}

	void run(std::string args)
	{
		thing res;
		parse_args<0, Arguments...>(args, res);
		if (!command_handler::parse_single_argument(args).empty())
		{
			// FIXME: needs an exception for situation where too many arguments were passed
		}
		exec(res);
	}

	Command(Arguments... args) : args{args...}
	{
	}

	virtual ~Command() = default;
};

int main()
{
	Command x(Argument<StringArgument>{"hi"});
	CommandGeneric &h = x;
	h.run("hello hello");
}
