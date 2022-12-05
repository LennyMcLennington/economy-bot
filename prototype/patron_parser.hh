#ifndef PATRON_PARSER_HH
#define PATRON_PARSER_HH
#include <string>
#include <cctype>
namespace patron::parser
{
namespace
{
enum class quotation
{
	not_quoted,
	double_quoted,
	single_quoted
};
} // namespace

inline std::string parse_single_argument(std::string &content)
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
#endif // PATRON_PARSER_HH
