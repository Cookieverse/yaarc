#ifndef YAARC_STRINGTOINT64_HPP
#define YAARC_STRINGTOINT64_HPP
namespace yaarc::impl {
	int64_t StringToInt64(const char* start, const char* end, bool& success) {
		success = false;

		auto str = start;
		// skip spaces
		while (isspace(*str)) {
			str++;
			if (str >= end) {
				return 0;
			}
		}

		bool neg = false;
		switch (*str) {
			case '-':
				neg = true;
				/* fall through */
			case '+':
				str++;
			default:
				break;
		}
		if (str >= end) {
			return 0;
		}

		// skip spaces
		while (isspace(*str)) {
			str++;
			if (str >= end) {
				return 0;
			}
		}

		int64_t val = 0;

		bool had_space = false;
		while (str < end) {
			if (!isdigit(*str)) {
				// skip trailing spaces
				if (isspace(*str)) {
					had_space = true;
					str++;
					continue;
				}
				return 0;
			}
			// if we encountered a space we assume the string has ended,
			// but if we find a number after a space, the string is invalid
			if (had_space) {
				return 0;
			}
			val = (10 * val) + (*str++ - '0');
		}

		success = true;
		if (neg) {
			return -val;
		}
		return val;
	}

	int64_t StringToInt64(const std::string& string, bool& success) {
		return StringToInt64(string.data(), string.data() + string.size(), success);
	}

	int64_t StringToInt64(const std::string_view& string, bool& success) {
		return StringToInt64(string.data(), string.data() + string.size(), success);
	}
}
#endif //YAARC_STRINGTOINT64_HPP
