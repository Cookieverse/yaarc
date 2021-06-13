#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"
#ifndef YAARC_VALUE_HPP
#define YAARC_VALUE_HPP

#include <new>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <fmt/format.h>
#include "impl/StringToInt64.hpp"

namespace yaarc {
	enum class ValueType {
		Null,
		String,
		Error,
		Integer,
		Array
	};

	class Value {
		ValueType m_type;
		union {
			int64_t m_int;
			// strings are stored as uint8_t vectors and just cast when we require a string
			std::vector<uint8_t> m_string;
			std::vector<Value> m_array;
		};

	public:
		Value() {
			m_type = ValueType::Null;
		}

		// creating a string
		Value(const char* c_string) : m_type(ValueType::String) {
			auto data = reinterpret_cast<const uint8_t*>(c_string);
			new(&m_string) std::vector<uint8_t>;
			m_string.insert(m_string.begin(), data, data + strlen(c_string));
		}

		Value(const char* string, size_t len) : m_type(ValueType::String) {
			auto data = reinterpret_cast<const uint8_t*>(string);
			new(&m_string) std::vector<uint8_t>;
			m_string.insert(m_string.begin(), data, data + len);
		}

		Value(const uint8_t* data, size_t len) : m_type(ValueType::String) {
			new(&m_string) std::vector<uint8_t>;
			m_string.insert(m_string.begin(), data, data + len);
		}

		Value(const std::string& string) : m_type(ValueType::String) {
			auto data = reinterpret_cast<const uint8_t*>(string.data());
			new(&m_string) std::vector<uint8_t>;
			m_string.insert(m_string.begin(), data, data + string.length());
		}

		Value(const std::string_view& string) : m_type(ValueType::String) {
			auto data = reinterpret_cast<const uint8_t*>(string.data());
			new(&m_string) std::vector<uint8_t>;
			m_string.insert(m_string.begin(), data, data + string.length());
		}

		// creating an integer
		Value(int64_t integer) : m_type(ValueType::Integer), m_int(integer) {
		}

		// array
		Value(std::vector<Value> values) : m_type(ValueType::Array) {
			new(&m_array) std::vector<Value>;
			m_array = std::move(values);
		}
		Value(std::initializer_list<Value> values) : m_type(ValueType::Array) {
			new(&m_array) std::vector<Value>(values);
		}

		Value(ValueType type) : m_type(type) {
			switch (type) {
				case ValueType::String:
				case ValueType::Error:
					new(&m_string) std::vector<uint8_t>;
					break;
				case ValueType::Integer:
					m_int = 0;
					break;
				case ValueType::Array:
					new(&m_array) std::vector<Value>;
					break;
				case ValueType::Null:
					break;
			}
		}

		// copy
		Value(const Value& other) : m_type(ValueType::Null) {
			*this = other;
		}

		Value& operator=(const Value& other) {
			// destroy any potential old things if there's a different type
			bool needsInit = false;
			if (m_type != other.m_type) {
				needsInit = true;
				this->~Value();
			}
			m_type = other.m_type;
			switch (m_type) {
				case ValueType::String:
				case ValueType::Error:
					if (needsInit) {
						new(&m_string) std::vector<uint8_t>;
					}
					m_string = other.m_string;
					break;
				case ValueType::Integer:
					m_int = other.m_int;
					break;
				case ValueType::Array:
					if (needsInit) {
						new(&m_array) std::vector<Value>;
					}
					m_array = other.m_array;
					break;
				case ValueType::Null:
					break;
			}
			return *this;
		}

		// move
		Value(Value&& other) noexcept {
			m_type = other.m_type;
			switch (m_type) {
				case ValueType::String:
				case ValueType::Error:
					new(&m_string) std::vector<uint8_t>;
					m_string.swap(other.m_string);
					break;
				case ValueType::Integer:
					m_int = other.m_int;
					break;
				case ValueType::Array:
					new(&m_array) std::vector<Value>;
					m_array.swap(other.m_array);
					break;
				case ValueType::Null:
					break;
			}
		}

		~Value() {
			switch (m_type) {
				case ValueType::String:
				case ValueType::Error:
					m_string.~vector();
					break;
				case ValueType::Array:
					m_array.~vector();
					break;
				case ValueType::Integer:
				case ValueType::Null:
					break;
			}
			m_type = ValueType::Null;
		}

		bool operator==(const Value& rhs) const {
			if (m_type != rhs.m_type) {
				return false;
			}
			switch (m_type) {
				case ValueType::Null:
					return true;
				case ValueType::String:
				case ValueType::Error:
					return m_string == rhs.m_string;
				case ValueType::Integer:
					return AsInteger() == rhs.AsInteger();
				case ValueType::Array:
					if (m_array.size() != rhs.m_array.size()) {
						return false;
					}
					for (size_t i = 0; i < m_array.size(); i++) {
						if (m_array[i] != rhs.m_array[i]) {
							return false;
						}
					}
					return true;
			}
			throw std::runtime_error(fmt::format("Bad ValueType {}", m_type));
		}

		bool operator!=(const Value& rhs) const {
			return !(rhs == *this);
		}

		[[nodiscard]] ValueType GetType() const {
			return m_type;
		}

		[[nodiscard]] bool IsString() const {
			return m_type == ValueType::String;
		}

		[[nodiscard]] std::string_view AsString() const {
			if (m_type != ValueType::String && m_type != ValueType::Error) {
				throw std::runtime_error(fmt::format("Cannot convert type {} to string", m_type));
			}
			return std::string_view(reinterpret_cast<const char*>(m_string.data()), m_string.size());
		}

		const std::vector<uint8_t> AsBytes() const {
			if (m_type != ValueType::String && m_type != ValueType::Error) {
				throw std::runtime_error(fmt::format("Cannot convert type {} to string", m_type));
			}
			return m_string;
		}

		[[nodiscard]] std::string AsStringCopy() const {
			if (m_type != ValueType::String && m_type != ValueType::Error) {
				throw std::runtime_error(fmt::format("Cannot convert type {} to string", m_type));
			}
			return std::string(reinterpret_cast<const char*>(m_string.data()), m_string.size());
		}

		[[nodiscard]] bool IsNull() const {
			return m_type == ValueType::Null;
		}

		[[nodiscard]] bool IsInteger(bool checkConvertible = true) const {
			if (m_type == ValueType::Integer) {
				return true;
			}
			if (IsString()) {
				auto data = reinterpret_cast<const char*>(m_string.data());
				if (data != nullptr) {
					bool isInteger;
					impl::StringToInt64(data, data + m_string.size(), isInteger);
					return isInteger;
				}
			}
			return false;
		}

		[[nodiscard]] int64_t AsInteger() const {
			if (m_type == ValueType::Integer) {
				return m_int;
			}
			if (IsString()) {
				auto data = reinterpret_cast<const char*>(m_string.data());
				if (data != nullptr) {
					bool isInteger;
					int64_t res = impl::StringToInt64(data, data + m_string.size(), isInteger);
					if (isInteger) {
						return res;
					}
				}
				throw std::runtime_error(
						fmt::format("Could not convert string '{}' to an integer",
									std::string_view(data, m_string.size())));
			}
			throw std::runtime_error(fmt::format("Cannot convert type {} to integer", m_type));
		}


		[[nodiscard]] bool IsArray() const {
			return m_type == ValueType::Array;
		}

		[[nodiscard]] const std::vector<Value>& AsArray() const {
			if (m_type != ValueType::Array) {
				throw std::runtime_error(fmt::format("Cannot convert type {} to array", m_type));
			}
			return m_array;
		}

		[[nodiscard]] bool IsError() const {
			return m_type == ValueType::Error;
		}

		[[nodiscard]] std::string_view AsError() const {
			if (m_type != ValueType::Error) {
				throw std::runtime_error(fmt::format("Cannot convert type {} to error", m_type));
			}
			return AsString();
		}

		template<typename T>
		inline T As() const;

		[[nodiscard]]
		/// Errors are pretty much string types under the hood, so there can't be a constructor overload
		static Value NewError(const void* data, size_t size) {
			auto v = Value(static_cast<const uint8_t*>(data), size);
			v.m_type = ValueType::Error;
			return std::move(v);
		}
	};

	template<>
	inline std::string_view Value::As<std::string_view>() const {
		return AsString();
	};

	template<>
	inline std::string Value::As<std::string>() const {
		return AsStringCopy();
	};

	template<>
	inline int64_t Value::As<int64_t>() const {
		return AsInteger();
	};

	template<>
	inline std::vector<Value> Value::As<std::vector<Value>>() const {
		return AsArray();
	};
}

#endif //YAARC_VALUE_HPP

#pragma clang diagnostic pop