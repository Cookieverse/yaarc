#ifndef YAARC_VALUEFORMATTER_HPP
#define YAARC_VALUEFORMATTER_HPP

#include "Value.hpp"
#include <fmt/format.h>

template<>
struct fmt::formatter<yaarc::Value> : fmt::formatter<string_view> {
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template<typename FormatContext>
	auto format(const yaarc::Value& value, FormatContext& ctx) {
		// auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
		// ctx.out() is an output iterator to write to.
		switch (value.GetType()) {
			case yaarc::ValueType::Null:
				return format_to(
						ctx.out(),
						"Value(Null)");
			case yaarc::ValueType::String:
				return format_to(
						ctx.out(),
						"Value(String, \"{}\")", value.AsString());
			case yaarc::ValueType::Error:
				return format_to(
						ctx.out(),
						"Value(Error, \"{}\")", value.AsError());
			case yaarc::ValueType::Integer:
				return format_to(
						ctx.out(),
						"Value(Integer, {})", value.AsInteger());
			case yaarc::ValueType::Array:
				return format_to(
						ctx.out(),
						"Value(Array, [{}])", fmt::join(value.AsArray(), ","));
		}
		throw std::runtime_error("Unknown value type");
	}
};

template<>
struct fmt::formatter<yaarc::ValueType> : fmt::formatter<string_view> {
	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template<typename FormatContext>
	auto format(const yaarc::ValueType& value, FormatContext& ctx) {
		// auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
		// ctx.out() is an output iterator to write to.
		switch (value) {
			case yaarc::ValueType::Null:
				return format_to(
						ctx.out(),
						"Null");
			case yaarc::ValueType::String:
				return format_to(
						ctx.out(),
						"String");
			case yaarc::ValueType::Error:
				return format_to(
						ctx.out(),
						"Error");
			case yaarc::ValueType::Integer:
				return format_to(
						ctx.out(),
						"Integer");
			case yaarc::ValueType::Array:
				return format_to(
						ctx.out(),
						"Array");
		}
		throw std::runtime_error("Unknown value type");
	}
};

#endif //YAARC_VALUEFORMATTER_HPP
