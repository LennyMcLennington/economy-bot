#ifndef PATRON_CMDS_HH
#define PATRON_CMDS_HH

#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <limits>
#include <concepts>
#include <optional>

#include "patron_parser.hh"

namespace patron::cmds
{
namespace readers
{
using String = decltype([](const std::string s) { return s; });
using Double = decltype([](const std::string &s) { return std::stod(s); });
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

template <typename T>
concept ValidCommandOptions = requires
{
	{T::required_arguments} -> std::same_as<std::size_t>;
};

template <typename... CommandContextTypes> class CommandHandler
{
public:
	std::string prefix = "$";
	CommandHandler() = default;
	virtual ~CommandHandler() = default;

	CommandHandler(std::string prefix)
	{
	}

	virtual std::string::size_type check_prefix(std::string cmd) const
	{
		if (cmd.starts_with(prefix))
		{
			return prefix.length();
		}
		return std::string::npos;
	};

	class CommandInterface;
	std::unordered_map<std::string, std::shared_ptr<CommandInterface>> m_commands;
	void handle_command(CommandContextTypes... context, std::string command) const
	{
		std::string::size_type prefix_length = check_prefix(command);
		if (prefix_length == std::string::npos)
			return;
		command = command.substr(prefix_length);
		std::string command_name = patron::parser::parse_single_argument(command);
#ifndef NDEBUG
		std::cout << "Running command... \"" << command_name << "\"\n";
#endif
		if (!m_commands.contains(command_name))
			return;
		{
			m_commands.at(command_name)->run(context..., command);
		}
	}

	void register_command(std::shared_ptr<CommandInterface> cmd)
	{
		for (std::string name : cmd->aliases())
		{
#ifndef NDEBUG
			std::cout << "Adding alias: " << name << "\n";
#endif
			if (!m_commands.contains(name))
			{
				m_commands[name] = cmd;
			}
		}
	}
	template <std::size_t option>
	void register_command(auto... args)
	{
		register_command(
			std::shared_ptr<CommandInterface>{new Command{std::integral_constant<std::size_t, option>{}, args...}});
	}
	template <std::size_t option>
	void register_command(std::initializer_list<std::string> &&a1, auto... args)
	{
		register_command(std::shared_ptr<CommandInterface>{
			new Command{std::integral_constant<std::size_t, option>{}, a1, args...}});
	}

	struct CommandOptions
	{
		std::size_t args_required = std::numeric_limits<size_t>::max();
	};

	class CommandInterface
	{
	public:
		virtual void run(CommandContextTypes..., std::string args) const = 0;
		virtual std::span<const std::string> aliases() const = 0;
		virtual std::string_view desc() const = 0;
		virtual std::string_view category() const = 0;
		virtual const CommandOptions &opts() const = 0;
		virtual ~CommandInterface() = default;
	};

	template <std::size_t, typename... T> class Command;

	template <std::size_t required_args, typename... ArgReaders> class Command<required_args, std::tuple<ArgReaders...>> : public CommandInterface
	{
	public:
		using run_fn_type = std::function<void(CommandContextTypes..., typename Argument<ArgReaders>::result_type...)>;
		using args_type = std::tuple<typename Argument<ArgReaders>::result_type...>;
		using args_with_optional_wrappers = decltype([]<std::size_t... required_indices, std::size_t... optional_indices>(std::index_sequence<required_indices...>, std::index_sequence<optional_indices...>, args_type a) {
			return std::make_tuple(std::get<required_indices>(a)..., std::make_optional(std::get<optional_indices>(a))...);
		}(std::make_index_sequence<required_args>(), std::make_index_sequence<std::tuple_size_v<args_type> - required_args>(), std::declval<args_type>()));
		using arginfo_type = std::tuple<Argument<ArgReaders>...>;

		std::vector<std::string> m_aliases;
		std::string m_desc;
		std::string m_category;
		CommandOptions m_opts;

		arginfo_type m_arginfo;
		run_fn_type m_run_fn;

		std::span<const std::string, std::dynamic_extent> aliases() const override
		{
			return m_aliases;
		}

		std::string_view desc() const override
		{
			return m_desc;
		}
		std::string_view category() const override
		{
			return m_category;
		}
		const CommandOptions &opts() const override
		{
			return m_opts;
		}

		template <std::size_t... Indexes>
		args_type parse_args(std::string &args, std::integer_sequence<std::size_t, Indexes...>) const
		{
			return args_type{typename std::remove_reference_t<decltype(std::get<Indexes>(m_arginfo))>::reader{}(
				patron::parser::parse_single_argument(args))...};
		}

		const std::vector<std::reference_wrapper<ArgumentInterface>> args_vec{std::apply(
			[](auto &&...args) { return std::vector<std::reference_wrapper<ArgumentInterface>>{args...}; }, m_arginfo)};
		const std::vector<std::reference_wrapper<ArgumentInterface>> &args()
		{
			return args_vec;
		}

		virtual void run(CommandContextTypes... extra_args, std::string content) const override
		{
			args_type res = parse_args(content, std::make_index_sequence<std::tuple_size_v<decltype(m_arginfo)>>{});
			if (!patron::parser::parse_single_argument(content).empty())
			{
			  // TODO: handle the situation where too many or few arguments were passed
			}
			std::apply(m_run_fn, std::tuple_cat(std::make_tuple(extra_args...), res));
		}

		Command(std::integral_constant<std::size_t, required_args>, std::string name, auto &&...args) : Command(std::initializer_list<std::string>{name}, args...)
		{
		}
		Command(std::integral_constant<std::size_t, required_args>, std::initializer_list<std::string> aliases, std::string desc, std::string category, CommandOptions opts,
				arginfo_type args, run_fn_type f)
			: m_aliases{aliases}, m_desc{desc}, m_category{category}, m_arginfo{args}, m_run_fn{f}
		{
		}
	};

#define common_opts auto &&, auto &&, CommandOptions, std::tuple<Argument<ArgTypes>...>, auto &&

	template <std::size_t size, typename... ArgTypes> Command(std::integral_constant<std::size_t, size>, auto &&, common_opts) -> Command<size, std::tuple<ArgTypes...>>;

	template <std::size_t size, typename... ArgTypes>
	Command(std::integral_constant<std::size_t, size>, std::initializer_list<std::string>, common_opts) -> Command<size, std::tuple<ArgTypes...>>;
};
} // namespace patron::cmds

#endif // PATRON_CMDS_HH
