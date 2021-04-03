#include <yaarc.hpp>
#include "yaarc_test_helper.hpp"

int main() {
	// Parse array
	const struct {
		yaarc::Value val;
		std::string res;
	} values[] = {
			{std::vector<yaarc::Value>{}, "*0\r\n"},
			{yaarc::Value(), "$-1\r\n"},
			{yaarc::Value("123456789"), "$9\r\n123456789\r\n"},
			{yaarc::Value::NewError("123456789", 9), "$9\r\n123456789\r\n"},
			{yaarc::Value(123456789), "$9\r\n123456789\r\n"},
			{yaarc::Value(-123456789), "$10\r\n-123456789\r\n"},
	};

	size_t c = 0;
	for (auto& val : values) {
		std::vector<uint8_t> output;
		try {
			yaarc::EncodeRESPBulkString(val.val, output);
		} catch (std::runtime_error& e) {
			TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
		}
		if (output.size() != val.res.size() || memcmp(output.data(), val.res.data(), output.size()) != 0) {
			TEST_ERROR(fmt::format("Unexpected output: {}, expected {} ",
								   std::string_view(reinterpret_cast<const char*>(output.data()), output.size()),
								   val.res));
		}
		c++;
	}
}