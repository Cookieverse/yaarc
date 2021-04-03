#ifndef YAARC_YAARC_HPP
#define YAARC_YAARC_HPP
#include <memory>
#include <iostream>
#include <utility>
#include "impl/Client.hpp"
#include "Config.hpp"
#include "Value.hpp"

namespace yaarc {
	class Yaarc {
	private:
		std::shared_ptr<impl::Client> m_impl;
	public:
		explicit Yaarc(asio::io_context& io, Config config) : m_impl(std::make_shared<impl::Client>(io, std::move(config))) {
			SetErrorLogger([](LogLevel level, std::string msg){
				std::cout << "[YAARC][" << LogLevelToString(level) << "] " << msg << std::endl;
			});
			m_impl->Connect();
		}

		void SetErrorLogger(std::function<void(LogLevel, std::string)> handler) {
			m_impl->SetLogger(std::move(handler));
		}

		void Get(std::string_view key, std::function<void(std::error_code, Value)> callback) {
			m_impl->Cmd({"GET", key}, std::move(callback));
		}
	};
}

#endif //YAARC_YAARC_HPP
