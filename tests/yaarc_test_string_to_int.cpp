#include "yaarc_test_helper.hpp"
#include <yaarc.hpp>
#include <vector>
int main() {
	struct test {
		std::string string;
		bool valid;
		int64_t value;
	};

	std::vector<test> values {
			// basic tests
			{"123", true, 123},
			{"1234", true, 1234},
			{"12345678910111213141516", true, 1234},
			{"0", true, 0},
			// signed
			{"+123", true, 123},
			{"+1234", true, 1234},
			{"+12345678910111213141516", true, 1234},
			{"+0", true, 0},
			{"-123", true, -123},
			{"-1234", true, -1234},
			{"-12345678910111213141516", true, -1234},
			{"-0", true, 0}, // -0 edge case
			// leading/trailing spaces
			{" 123", true, 123},
			{" -123", true, -123},
			{"     123", true, 123},
			{"     -123", true, -123},
			{"123 ", true, 123},
			{"-123 ", true, -123},
			{"123     ", true, 123},
			{"-123     ", true, -123},
			{"     123 ", true, 123},
			{"     -123 ", true, -123},
			{"     123     ", true, 123},
			{"     -123     ", true, -123},
			{" + 123", true, 123},
			{" - 123", true, -123},
			{"     +   123", true, 123},
			{"     -   123", true, -123},
			{"+ 123 ", true, 123},
			{"- 123 ", true, -123},
			{"+   123     ", true, 123},
			{"-   123     ", true, -123},
			{"     + 123 ", true, 123},
			{"     - 123 ", true, -123},
			{"     +   123     ", true, 123},
			{"     -   123     ", true, -123},
			// invalid strings
			{"", false},
			{"++0", false},
			{"--1", false},
			{"+-1", false},
			{"-+1", false},
			{"1 2 3", false},
			{" 1 2 3 ", false},
			{"hello", false},
			{"five", false},
			{"0x123", false},
			{"0b101010", false},
			{"1-", false},
			{"1+", false},
			{" ", false},
			{"-", false},
			{"+", false},
	};
	size_t i = 0;
	for (const auto& v : values) {
		bool success;
		// test string api
		auto value = yaarc::impl::StringToInt64(v.string, success);
		TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), success, v.valid);
		if (v.valid) {
			TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), value, value);
		}
		// test string_view api
		value = yaarc::impl::StringToInt64(std::string_view(v.string), success);
		TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), success, v.valid);
		if (v.valid) {
			TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), value, value);
		}
		// test char* api
		value = yaarc::impl::StringToInt64(v.string.data(), v.string.data() + v.string.size(), success);
		TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), success, v.valid);
		if (v.valid) {
			TEST_ASSERT_HINT(fmt::format("#{} '{}'", i, v.string), value, value);
		}
		i++;
	}
}
