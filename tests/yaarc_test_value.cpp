#include "yaarc_test_helper.hpp"
#include <yaarc.hpp>
#include <vector>

template<>
struct fmt::formatter<std::vector<yaarc::Value>> : fmt::formatter<string_view> {
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template<typename FormatContext>
	auto format(const std::vector<yaarc::Value>& values, FormatContext& ctx) {
		// auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
		// ctx.out() is an output iterator to write to.
		return format_to(ctx.out(), "[{}]", fmt::join(values, ","));
	}
};

void TestType(yaarc::ValueType type);

template<typename T>
void Test(T source, T unequal) {
	{
		// construction from value
		yaarc::Value value(source);
		TEST_ASSERT_HINT(typeid(T).name(), value.As<T>(), source);
		// test if basic comparisons work
		TEST_ASSERT_HINT(typeid(T).name(), (value == value), true);
		TEST_ASSERT_HINT(typeid(T).name(), (value != value), false);
		yaarc::Value valueUnequal(unequal);
		TEST_ASSERT_HINT(typeid(T).name(), (value == valueUnequal), false);
		TEST_ASSERT_HINT(typeid(T).name(), (value != valueUnequal), true);
	}
	{
		// assignment from another value
		yaarc::Value value(source);
		yaarc::Value other;
		other = value;
		TEST_ASSERT_HINT(typeid(T).name(), other.As<T>(), source);
	}
	{
		// copy assignment
		yaarc::Value value(source);
		yaarc::Value other(value); // NOLINT(performance-unnecessary-copy-initialization)

		TEST_ASSERT_HINT(typeid(T).name(), other.As<T>(), source);
	}
	{
		// move assignment
		yaarc::Value value(source);
		yaarc::Value other(std::move(value));

		TEST_ASSERT_HINT(typeid(T).name(), other.As<T>(), source);
	}
}


void TestType(yaarc::ValueType type) {
	yaarc::Value value(type);
	TEST_ASSERT_HINT("Testing Null", value.IsNull(), (type == yaarc::ValueType::Null));
	TEST_ASSERT_HINT("Testing String", value.IsString(), (type == yaarc::ValueType::String));
	TEST_ASSERT_HINT("Testing Error", value.IsError(), (type == yaarc::ValueType::Error));
	TEST_ASSERT_HINT("Testing Integer", value.IsInteger(), (type == yaarc::ValueType::Integer));
	TEST_ASSERT_HINT("Testing Array", value.IsArray(), (type == yaarc::ValueType::Array));
	bool exception = false;
	try {
		value.AsInteger();
	} catch (std::runtime_error& e) {
		exception = true;
		if (value.IsInteger()) {
			TEST_ERROR(fmt::format("Did not expect AsInteger exception for type {}: {}", type, e.what()));
		}
	}
	if (!exception && !value.IsInteger()) {
		TEST_ERROR(fmt::format("Expected AsInteger exception for type {}, but did not get one", type));
	}
	exception = false;
	try {
		value.AsString();
	} catch (std::runtime_error& e) {
		exception = true;
		if (value.IsString() || value.IsError()) {
			TEST_ERROR(fmt::format("Did not expect AsString exception for type {}: {}", type, e.what()));
		}
	}
	if (!exception && !(value.IsString() || value.IsError())) {
		TEST_ERROR(fmt::format("Expected AsString exception for type {}, but did not get one", type));
	}
	exception = false;
	try {
		value.AsStringCopy();
	} catch (std::runtime_error& e) {
		exception = true;
		if (value.IsString() || value.IsError()) {
			TEST_ERROR(fmt::format("Did not expect AsStringCopy exception for type {}: {}", type, e.what()));
		}
	}
	if (!exception && !(value.IsString() || value.IsError())) {
		TEST_ERROR(fmt::format("Expected AsStringCopy exception for type {}, but did not get one", type));
	}
	exception = false;
	try {
		value.AsArray();
	} catch (std::runtime_error& e) {
		exception = true;
		if (value.IsArray()) {
			TEST_ERROR(fmt::format("Did not expect AsArray exception for type {}: {}", type, e.what()));
		}
	}
	if (!exception && !value.IsArray()) {
		TEST_ERROR(fmt::format("Expected AsArray exception for type {}, but did not get one", type));
	}
	exception = false;
	try {
		value.AsError();
	} catch (std::runtime_error& e) {
		exception = true;
		if (value.IsError()) {
			TEST_ERROR(fmt::format("Did not expect AsError exception for type {}: {}", type, e.what()));
		}
	}
	if (!exception && !value.IsError()) {
		TEST_ERROR(fmt::format("Expected AsError exception for type {}, but did not get one", type));
	}
}


int main() {
	{
		std::string string("Test 123");
		{
			// construction from c string
			yaarc::Value value(string.c_str());
			TEST_ASSERT(value.AsString(), string);
			TEST_ASSERT(value.AsStringCopy(), string);
		}
		{
			// construction from c string with length
			yaarc::Value value(string.c_str(), string.size());
			TEST_ASSERT(value.AsString(), string);
			TEST_ASSERT(value.AsStringCopy(), string);
		}
		Test(string, std::string("Not Test 123"));
		std::string_view sv(string);
		Test(sv, std::string_view("Not Test 123"));
		int64_t integer = 123456789;
		int64_t ninteger = -123456789;
		Test(integer, ninteger);
		Test(ninteger, integer);
		std::vector<yaarc::Value> values;
		std::vector<yaarc::Value> values2;
		values2.emplace_back(string);
		values2.emplace_back(integer);
		values2.emplace_back(ninteger);
		values2.emplace_back(values);
		Test(values, values2);
		values.emplace_back(string);
		values.emplace_back(integer);
		values.emplace_back(ninteger);
		Test(values, values2);
		values.emplace_back(values);
		Test(values, values2);
	}
	// test null values
	{
		{
			// construction from value
			yaarc::Value value;
			TEST_ASSERT(value.IsNull(), true);
		}
		{
			// assignment from another value
			yaarc::Value value;
			yaarc::Value other;
			other = value;
			TEST_ASSERT(other.IsNull(), true);
		}
		{
			// copy assignment
			yaarc::Value value;
			yaarc::Value other(value); // NOLINT(performance-unnecessary-copy-initialization)

			TEST_ASSERT(other.IsNull(), true);
		}
		{
			// move assignment
			yaarc::Value value;
			yaarc::Value other(std::move(value));

			TEST_ASSERT(other.IsNull(), true);
		}
	}

	// test types
	TestType(yaarc::ValueType::Null);
	TestType(yaarc::ValueType::String);
	TestType(yaarc::ValueType::Error);
	TestType(yaarc::ValueType::Integer);
	TestType(yaarc::ValueType::Array);

	std::vector<yaarc::Value> values;
	values.emplace_back(yaarc::ValueType::Null);
	values.emplace_back(yaarc::ValueType::String);
	values.emplace_back(yaarc::ValueType::Error);
	values.emplace_back(yaarc::ValueType::Integer);
	values.emplace_back(yaarc::ValueType::Array);
	for (size_t i = 0; i < values.size(); i++) {
		for (size_t o = 0; i < values.size(); i++) {
			if (i == o) {
				TEST_ASSERT(values[i], values[o]);
			} else {
				TEST_ASSERT_UNEQUAL(values[i], values[o]);
			}
		}
	}


}

