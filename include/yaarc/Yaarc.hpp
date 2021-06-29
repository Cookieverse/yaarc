#ifndef YAARC_YAARC_HPP
#define YAARC_YAARC_HPP

#include <memory>
#include <iostream>
#include <utility>
#include "impl/Client.hpp"
#include "Config.hpp"
#include "Value.hpp"
#include "SetOption.hpp"

namespace yaarc {
	class Yaarc {
	private:
		std::shared_ptr<impl::Client> m_impl;
	public:
		typedef std::function<void(std::error_code, Value)> Callback;

		explicit Yaarc(YAARC_ASIO::io_context& io, Config config) : m_impl(
				std::make_shared<impl::Client>(io, std::move(config))) {
			SetLogger([](LogLevel level, std::string msg) {
				std::cout << "[YAARC][" << LogLevelToString(level) << "] " << msg << std::endl;
			});
			m_impl->Connect();
		}

		template <typename Callback>
		void Shutdown(Callback handler) {
			m_impl->Stop(handler);
		}

		void SetLogger(std::function<void(LogLevel, std::string)> handler) {
			m_impl->SetLogger(std::move(handler));
		}

		void Get(std::string_view key, Callback callback) {
			m_impl->Cmd({"GET", key}, std::move(callback));
		}

		/// Sets a key to the specified value
		/// \param key
		/// \param callback
		/// \param expires
		/// \param options
		void Set(std::string_view key, Value value, Callback callback, std::chrono::milliseconds expires = std::chrono::milliseconds (0), SetOption options = SetOption::Always) {
			m_impl->Cmd({"SET", key, std::move(value)}, std::move(callback));
		}

		/// Sets a key to the specified value
		/// \param key
		/// \param callback
		/// \param expires
		/// \param options
		void Set(std::string_view key, Value value, std::chrono::milliseconds expires = std::chrono::milliseconds (0), SetOption options = SetOption::Always) {
			m_impl->Cmd({"SET", key, std::move(value)}, {});
		}

		void Cmd(const Value& value, Callback callback) {
			m_impl->Cmd(value, std::move(callback));
		}
	};
}

#endif //YAARC_YAARC_HPP
