#include <yaarc.hpp>
#include "yaarc_test_helper.hpp"

int main() {

	// common vars
	size_t consumed;
	yaarc::Value val;
	size_t c = 0;

	// Parse a simple string
	const struct {
		std::string str;
		std::string res;
		bool valid;
	} simple_strings[] = {
			{"+OK\r\n", "OK", true},
			{"+OK\ra",  "OK", false},
			{"+\r\n",   "",   true},
	};

	for (auto& str : simple_strings) {
		bool caught = false;
		try {
			TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), str.str.size(), consumed, val), str.valid);
		} catch (std::runtime_error& e) {
			caught = true;
			if (str.valid) {
				TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
			}
		}
		if (str.valid) {
			TEST_ASSERT_HINT(c, consumed, str.str.size());
			TEST_ASSERT_HINT(c, val.IsString(), true);
			TEST_ASSERT_HINT(c, val.AsString(), str.res);
			// check if it will return false if there's not enough data
			for (int i = 0; i < str.str.size(); i++) {
				TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), i, consumed, val), false);
			}
		} else {
			TEST_ASSERT_HINT(c, caught, true);
		}

		c++;
	}

	// parse error
	const struct {
		std::string str;
		std::string res;
		bool valid;
	} error_strings[] = {
			{"-ERROR\r\n", "ERROR", true},
			{"-ERROR\ra",  "ERROR", false},
			{"-\r\n",      "",      true},
	};
	c = 0;

	for (auto& str : error_strings) {
		bool caught = false;
		try {
			TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), str.str.size(), consumed, val), str.valid);
		} catch (std::runtime_error& e) {
			caught = true;
			if (str.valid) {
				TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
			}
		}
		if (str.valid) {
			TEST_ASSERT_HINT(c, consumed, str.str.size());
			TEST_ASSERT_HINT(c, val.IsError(), true);
			TEST_ASSERT_HINT(c, val.AsError(), str.res);
			// check if it will return false if there's not enough data
			for (int i = 0; i < str.str.size(); i++) {
				TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), i, consumed, val), false);
			}
		} else {
			TEST_ASSERT_HINT(c, caught, true);
		}

		c++;
	}

	// Parse integer
	const struct {
		std::string string;
		int64_t value;
		bool validParse;
		bool validInt;
	} integer_strings[] = {
			{":1234\r\n",   1234,  true,  true},
			{":-1234\r\n",  -1234, true,  true},
			{":12a34\r\n",  1234,  false, false},
			// convertible to int
			{"+1234\r\n",   1234,  true,  true},
			{"+-1234\r\n",  -1234, true,  true},
			// convertible to int
			{"+12a345\r\n", 1234,  true,  false},
	};
	c = 0;
	for (auto str : integer_strings) {
		bool caught = false;
		try {
			TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.string.data(), str.string.size(), consumed, val), str.validParse);
		} catch (std::runtime_error& e) {
			caught = true;
			if (str.validParse) {
				TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
			}
		}
		if (str.validParse) {
			TEST_ASSERT_HINT(c, consumed, str.string.size());
			TEST_ASSERT_HINT(c, val.IsInteger(), str.validInt);
			if (str.validInt) {
				TEST_ASSERT_HINT(c, val.AsInteger(), str.value);
				// check if it will return false if there's not enough data
				for (int i = 0; i < str.string.size(); i++) {
					TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.string.data(), i, consumed, val), false);
				}
			}
		} else {
			TEST_ASSERT_HINT(c, caught, true);
		}

		c++;
	}

	// Parse a bulk string
	const struct {
		std::string str;
		std::string res;
		bool valid;
		bool isNull = false;
	} bulk_strings[] = {
			{"$2\r\nOK\r\n",        "OK",     true},
			{"$1\r\nOK\r\n",        "OK",     false},
			{"$3\r\nOK\r\n",        "OK",     false},
			{"$0\ra",               "OK",     false},
			{"$a\r\n",              "OK",     false},
			{"$0\r\n\r\n",          "",       true},
			{"$2\r\n\r\n\r\n",      "\r\n",   true},
			{"$4\r\na\r\nb\r\n",    "a\r\nb", true},
			{"$9999\r\na\r\nb\r\n", "a\r\nb", false},
			{"$-1\r\n",             "",       true, true},
	};

	c = 0;
	for (auto& str : bulk_strings) {
		try {
			TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), str.str.size(), consumed, val), str.valid);
		} catch (std::runtime_error& e) {
			if (str.valid) {
				TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
			}
		}
		if (str.valid) {
			TEST_ASSERT_HINT(c, consumed, str.str.size());
			if (str.isNull) {
				TEST_ASSERT_HINT(c, val.IsNull(), true);
			} else {
				TEST_ASSERT_HINT(c, val.IsString(), true);
				TEST_ASSERT_HINT(c, val.AsString(), str.res);
			}
			// check if it will return false if there's not enough data
			for (int i = 0; i < str.str.size(); i++) {
				TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), i, consumed, val), false);
			}
		}

		c++;
	}
	// Parse array
	const struct {
		std::string str;
		std::vector<yaarc::Value> res;
		bool valid;
		bool isNull;
	} array_strings[] = {
			{"*0\r\n", {}, true},
			{"*-1\r\n", {}, true, true},
			{"*1\r\n+OK\r\n", {"OK"}, true},
			{"*10\r\n+OK1\r\n+OK2\r\n+OK3\r\n+OK4\r\n+OK5\r\n+OK6\r\n+OK7\r\n+OK8\r\n+OK9\r\n+OK10\r\n", {"OK1","OK2","OK3","OK4","OK5","OK6","OK7","OK8","OK9","OK10",}, true},
	};

	c = 0;
	for (auto& str : array_strings) {
		try {
			TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), str.str.size(), consumed, val), str.valid);
		} catch (std::runtime_error& e) {
			if (str.valid) {
				TEST_ERROR(fmt::format("Unexpected exception: {} during {} ", e.what(), c));
			}
		}
		if (str.valid) {
			TEST_ASSERT_HINT(c, consumed, str.str.size());
			if (str.isNull) {
				TEST_ASSERT_HINT(c, val.IsNull(), true);
			} else {
				TEST_ASSERT_HINT(c, val.IsArray(), true);
				TEST_ASSERT_HINT(c, val, yaarc::Value(str.res));
			}
			// check if it will return false if there's not enough data
			for (int i = 0; i < str.str.size(); i++) {
				TEST_ASSERT_HINT(c, yaarc::ParseRESP(str.str.data(), i, consumed, val), false);
			}
		}

		c++;
	}
}