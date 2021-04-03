

#ifndef YAARC_PARSER_HPP
#define YAARC_PARSER_HPP
namespace yaarc::impl {
	bool ParseValue(const uint8_t* data, size_t length, size_t& consumed, Value& value);


	constexpr uint8_t RESP_SIMPLE_STRING = '+';
	constexpr uint8_t RESP_ERROR = '-';
	constexpr uint8_t RESP_INTEGER = ':';
	constexpr uint8_t RESP_BULK_STRING = '$';
	constexpr uint8_t RESP_ARRAY = '*';

	/// Attempts to find the end of the line in the provided range
	/// Returns nullptr if \r\n was not found
	/// Returns pointer for the start of the next line (1 char after \r\n) if line was found
	inline const uint8_t* FindLine(const uint8_t* start, const uint8_t* end) {
		for (auto c = start; c < end; c++) {
			if (*c == '\r') {
				if (c + 1 >= end) {
					// not enough data to find out if we have a \n after the \r
					return nullptr;
				}
				if (*(c + 1) != '\n') {
					throw std::runtime_error(
							fmt::format("While RESP parsing \r was not followed by \n, but {0} ({0:x})", *(c + 1)));
				}
				return c + 2;
			}
		}
		return nullptr;
	}

	inline bool ParseSimpleString(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		auto line = FindLine(data, data + length);
		if (!line) {
			return false;
		}
		if (line == data + 4) {
			value = Value("");
		} else {
			value = Value(data + 1, line - data - 3);
		}
		consumed = line - data;
		return true;
	}

	inline bool ParseError(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		auto line = FindLine(data, data + length);
		if (!line) {
			return false;
		}
		value = Value::NewError(data + 1, line - data - 3);
		consumed = line - data;
		return true;
	}

	inline bool ParseInteger(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		auto line = FindLine(data, data + length);
		if (!line) {
			return false;
		}
		bool success;
		auto res = StringToInt64(reinterpret_cast<const char*>(data + 1), reinterpret_cast<const char*>(line - 2),
								 success);
		if (!success) {
			throw std::runtime_error(fmt::format("Failed to parse RESP integer from string '{}'",
												 std::string_view(reinterpret_cast<const char*>(data + 1),
																  line - data - 3)));
		}
		value = Value(res);
		consumed = line - data;
		return true;
	}

	inline bool ParseBulkString(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		auto line = FindLine(data, data + length);
		if (!line) {
			return false;
		}
		bool success;
		auto stringLength = StringToInt64(reinterpret_cast<const char*>(data + 1),
										  reinterpret_cast<const char*>(line - 2), success);
		if (!success) {
			throw std::runtime_error(fmt::format("Failed to parse RESP bulk string length from string '{}'",
												 std::string_view(reinterpret_cast<const char*>(data + 1),
																  line - data - 3)));
		}
		// null
		if (stringLength < 0) {
			consumed = line - data;
			value = Value();
			return true;
		}
		auto end = data + length;
		if (line + stringLength + 2 > end) {
			return false;
		}
		if (line[stringLength] != '\r') {
			throw std::runtime_error(
					fmt::format("Failed to parse RESP bulk string - didn't end with \\r, but '{0}' {0:x}",
								line[stringLength + 1]));
		}
		if (line[stringLength + 1] != '\n') {
			throw std::runtime_error(
					fmt::format("Failed to parse RESP bulk string - didn't end with \\n, but '{0}' {0:x}",
								line[stringLength + 1]));
		}
		consumed = line + stringLength + 2 - data;
		value = Value(line, stringLength);
		return true;
	}

	inline bool ParseArray(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		auto line = FindLine(data, data + length);
		if (!line) {
			return false;
		}
		bool success;
		auto arrayLength = StringToInt64(reinterpret_cast<const char*>(data + 1),
										 reinterpret_cast<const char*>(line - 2), success);
		if (!success) {
			throw std::runtime_error(fmt::format("Failed to parse RESP bulk string length from string '{}'",
												 std::string_view(reinterpret_cast<const char*>(data + 1),
																  line - data - 3)));
		}
		// null
		if (arrayLength < 0) {
			consumed = line - data;
			value = Value();
			return true;
		}
		auto end = data + length;
		if (line > end) {
			return false;
		}
		std::vector<Value> values;
		values.reserve(arrayLength);
		for (int64_t i = 0; i < arrayLength; i++) {
			if (line >= end) {
				return false;
			}
			size_t used;
			Value v;
			if (!ParseValue(line, end - line, used, v)) {
				return false;
			}
			line += used;
			values.emplace_back(std::move(v));
		}
		consumed = line - data;
		value = Value(std::move(values));
		return true;
	}


	inline bool ParseValue(const uint8_t* data, size_t length, size_t& consumed, Value& value) {
		if (length == 0) {
			return false;
		}
		auto respType = static_cast<uint8_t>(data[0]);
		switch (respType) {
			case RESP_SIMPLE_STRING:
				return ParseSimpleString(data, length, consumed, value);
			case RESP_ERROR:
				return ParseError(data, length, consumed, value);
			case RESP_INTEGER:
				return ParseInteger(data, length, consumed, value);
			case RESP_BULK_STRING:
				return ParseBulkString(data, length, consumed, value);
			case RESP_ARRAY:
				return ParseArray(data, length, consumed, value);
			default:
				throw std::runtime_error(fmt::format("Unknown RESP type {}", respType));
		}
	}

	/// Encodes the provided input as a resp bulk string or an array of bulk strings
	inline void EncodeBulkString(const yaarc::Value& input, std::vector<uint8_t>& output) {
		switch (input.GetType()) {
			case ValueType::Null: {
				output.push_back(RESP_BULK_STRING);
				output.push_back('-');
				output.push_back('1');
				output.push_back('\r');
				output.push_back('\n');
				break;
			}
			case ValueType::String:
			case ValueType::Error: {
				output.push_back(RESP_BULK_STRING);
				auto length = std::to_string(input.AsString().size());
				output.insert(output.end(), length.begin(), length.end());
				output.push_back('\r');
				output.push_back('\n');
				output.insert(output.end(), input.AsString().begin(), input.AsString().end());
				output.push_back('\r');
				output.push_back('\n');
				break;
			}
			case ValueType::Integer: {
				output.push_back(RESP_BULK_STRING);
				auto str = std::to_string(input.AsInteger());
				auto length = std::to_string(str.size());
				output.insert(output.end(), length.begin(), length.end());
				output.push_back('\r');
				output.push_back('\n');
				output.insert(output.end(), str.begin(), str.end());
				output.push_back('\r');
				output.push_back('\n');
				break;
			}
			case ValueType::Array: {
				output.push_back(RESP_ARRAY);
				auto length = std::to_string(input.AsArray().size());
				output.insert(output.end(), length.begin(), length.end());
				output.push_back('\r');
				output.push_back('\n');
				for (auto& elem : input.AsArray()) {
					EncodeBulkString(elem, output);
				}
				break;
			}
		}
	}
}
#endif //YAARC_PARSER_HPP
