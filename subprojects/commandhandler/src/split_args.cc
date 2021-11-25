#include <command_handler/command.hh>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace command_handler {
enum class quotation { not_quoted, double_quoted, single_quoted };

std::vector<std::string> parse_arguments(command cmd, std::string_view content) {
	std::vector<std::string> output;

	// Starting in whitespace state ensures that emplace_back() will be called
	// as soon as the first non-whitespace character is found, since when we are
	// in non-whitespace mode it's assumed that the vector has had the
	// emplace_back() already called.
	bool whitespace_state = true;

	quotation quote_state = quotation::not_quoted;
	bool escape_state = false;

	for (auto it = content.begin(); it != content.end(); ++it) {
		if (std::isspace(*it) && quote_state == quotation::not_quoted && escape_state == false) {
			whitespace_state = true;
		} else {
			if (!escape_state && *it == '\\') {
				escape_state = true;
			} else {
				if (whitespace_state) {
					output.emplace_back();
					whitespace_state = false;
				}
				if (escape_state) {
					// Default is to ignore escaping by just outputting the backslash
					// back where it would've been in the original input.
					switch (*it) {
					default:
						output.back().push_back('\\');
					case '\\':
					case '"':
					case '\'':
					case ' ':
						output.back().push_back(*it);
					}
					escape_state = false;
				} else if (quote_state != quotation::single_quoted && *it == '"') {
					quote_state =
					    quote_state == quotation::not_quoted ? quotation::double_quoted : quotation::not_quoted;
				} else if (quote_state != quotation::double_quoted && *it == '\'') {
					quote_state =
					    quote_state == quotation::not_quoted ? quotation::single_quoted : quotation::not_quoted;
				} else {
					output.back().push_back(*it);
				}
			}
		}
	}

	return output;
}
} // namespace command_handler
