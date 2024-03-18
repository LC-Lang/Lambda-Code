#include "cli/cli.h"

cli::cli(int argc, const char** argv, const std::vector<Argument>& flags):
	m_argc(argc),
	m_argv(argv),
	m_flags(&flags),
  m_args(parse()) { }

bool cli::contains_flag(std::string_view flag) {
	auto flg = get_flag(flag);
	bool *ptr;

	if ((ptr = std::get_if<bool>(&flg)) == nullptr || *ptr != false) return true;
	return false;
}

std::string cli::get_flag_value(std::string_view flag) {
	return std::get<std::string>(get_flag(flag));
}

std::string cli::generate_help() {
	size_t max_width = 0;
	std::for_each(m_flags -> cbegin(), m_flags -> cend(), [&](const Argument& arg) {
		auto& [v1, v2] = arg.flags;
		if (v1.size() + v2.size() + 2 > max_width) max_width = v1.size() + v2.size() + 3;
	});

	std::string msg;
	for (const Argument& arg : *m_flags) {
		auto& [v1, v2] = arg.flags;
		msg += "\t" + v1 + ", " + v2;
		for (int c = 0; c < max_width - v1.size() - v2.size() - 2; c++) msg += ' ';
		msg += "     " + arg.desc + '\n';
	} return msg;
}

std::optional<std::string> cli::get_file() {
	for (int i = 1; i < m_argc; i++) {
		if (
			m_argv[i][0] != '-' &&
			(m_argv[i - 1][0] != '-' ||
			std::find_if(m_flags -> begin(), m_flags -> end(), [&](const Argument& _arg) {
				if (_arg.value_required) return false;
				auto &[v1, v2] = _arg.flags;
				return v1 == m_argv[i - 1] || v2 == m_argv[i - 1];
			}) != m_flags -> end()
		)) {
			return m_argv[i];
		}
	} return {};
}

std::vector<Argument> cli::parse() {
	std::vector<std::string_view> args(m_argv, m_argv + m_argc);
	std::vector<Argument> supplied;

	for (auto arg_iterator = args.cbegin() + 1; arg_iterator != args.cend(); ++arg_iterator) {
		// if the current argument starts with `-` sign, its a flag
		if (arg_iterator->starts_with("-")) {

			// check if the flag exists in the flag vector
			auto it = std::find_if(m_flags -> cbegin(), m_flags -> cend(), [&](const Argument& a) {
				auto& [v1, v2] = a.flags;
				return v1 == *arg_iterator || v2 == *arg_iterator;
			});

			if (it == m_flags -> cend()) { std::cerr << "Unknown flag: " << *arg_iterator << '\n'; exit(1); }

			if (!it->value_required) supplied.push_back(*it);
			else {
				if (arg_iterator + 1 == args.cend()) { std::cout << "Value missing for the flag: " << *arg_iterator << '\n'; exit(1); }

				Argument _arg = *it;
				_arg.value = *(arg_iterator + 1);
				supplied.push_back(_arg);
			}

		}
	}

	return supplied;
}

std::variant<std::string, bool> cli::get_flag(std::string_view flag) {
	auto it = std::find_if(m_args.cbegin(), m_args.cend(), [&](const Argument& a) {
		auto& [v1, v2] = a.flags;
		return v1 == flag || v2 == flag;
	});

	// Flag not found
	if (it == m_args.cend()) return false;

	// Flag exists
	if (!it->value_required) return true;

	return it->value;
}
