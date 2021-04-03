#ifndef YAARC_VALUEPARSER_HPP
#define YAARC_VALUEPARSER_HPP

#include <fmt/format.h>
#include "impl/Parser.hpp"

namespace yaarc {
	/// Parses a value from a resp response
	/// Returns false if more data is needed
	/// Throws std::runtime_error if the resp format is invalid
	inline bool ParseRESP(const void* data, size_t length, size_t& consumed, Value& value) {
		return impl::ParseValue(static_cast<const uint8_t*>(data), length, consumed, value);
	}

	/// Encodes the provided input as a resp bulk string
	inline void EncodeRESPBulkString(yaarc::Value input, std::vector<uint8_t>& output) {
		impl::EncodeBulkString(input, output);
	}
}
#endif //YAARC_VALUEPARSER_HPP
