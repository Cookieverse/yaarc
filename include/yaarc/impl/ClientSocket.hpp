#ifndef YAARC_CLIENTSOCKET_HPP
#define YAARC_CLIENTSOCKET_HPP

#include "IncludeAsio.hpp"
#include <utility>
#include "Command.hpp"

namespace yaarc::impl {
	class Client;

	const size_t ReadChunkSize = 8 * 1024; // TODO: move this somewhere better
	template<typename Executor>
	class ClientSocket
			: public std::enable_shared_from_this<ClientSocket<Executor>> {
		YAARC_ASIO::ip::tcp::socket m_socket;
		std::function<void(LogLevel, std::string)> m_logger;

		YAARC_ASIO::streambuf m_readBuffer;
		std::vector<uint8_t> m_writeBuffer;
		bool m_isWriting;
		std::vector<uint8_t> m_authBuffer;
		std::function<void(Value)> m_readHandler;
		std::function<void(YAARC_ERROR_CODE)> m_disconnectHandler;
	public:
		ClientSocket() = delete;

		ClientSocket(Executor io, std::function<void(Value)> read,
							  std::function<void(YAARC_ERROR_CODE)> disconnect) :
				m_socket(YAARC_ASIO::make_strand(io)),
				m_isWriting(false),
				m_readHandler(std::move(read)),
				m_disconnectHandler(std::move(disconnect)) {

		}

		auto Ptr() {
			return std::enable_shared_from_this<ClientSocket<Executor>>::shared_from_this();
		}

		void SetLogger(std::function<void(LogLevel, std::string)> handler) {
			m_logger = std::move(handler);
		}

		template<class Callback>
		void Stop(Callback handler) {
			YAARC_ASIO::dispatch(m_socket.get_executor(), [this, handler{std::move(handler)}, self{Ptr()}]() {
				YAARC_ERROR_CODE ec;
				if (m_socket.is_open()) {
					m_socket.close(ec);
					if (ec) {
						handler(ec);
					}
				}
				m_socket.cancel(ec);
				m_readHandler = nullptr;
				m_disconnectHandler = nullptr;
				handler(ec);
			});
		}

		template<typename Handler>
		void TryConnect(const YAARC_ASIO::ip::tcp::endpoint& endpoint, Handler handler) {
			YAARC_ASIO::dispatch(m_socket.get_executor(), [this, endpoint, handler{std::move(handler)}, self{Ptr()}]() {
				m_socket.async_connect(endpoint, [this, handler, endpoint, self{Ptr()}](const YAARC_ERROR_CODE& ec) {
					if (!ec) {
						Log(LogLevel::Info, fmt::format("Connected to {}:{}", endpoint.address().to_string(), endpoint.port()));
						StartRead();
					}
					handler(ec);
				});
			});
		}

		template<typename CommandRange, typename Handler>
		void StartWrite(CommandRange begin, CommandRange end, Handler handler) {
			if (m_isWriting) {
				throw std::runtime_error("StartWrite called while still writing");
			}
			m_isWriting = true;
			// m_writeBuffer is always fully written out
			m_writeBuffer.clear();
			for (auto& it = begin; begin != end; ++it) {
				m_writeBuffer.insert(m_writeBuffer.end(), it->Data.begin(), it->Data.end());
			}
			YAARC_ASIO::async_write(m_socket, YAARC_ASIO::const_buffer(m_writeBuffer.data(), m_writeBuffer.size()),
							  [this, self{Ptr()}, handler](YAARC_ERROR_CODE ec, size_t written) {
								  m_isWriting = false;
								  handler(ec);
								  if (ec) {
									  OnDisconnected(ec);
									  return;
								  }
							  });
		}

	private:
		void Log(LogLevel level, std::string error) {
			if (m_logger) {
				m_logger(level, std::move(error));
			}
		}

		void OnDisconnected(YAARC_ERROR_CODE ec) {
			if (m_disconnectHandler) {
				Log(LogLevel::Warning, fmt::format("Socket disconnected due to {} (#{})", ec.message(), ec.value()));
				m_disconnectHandler(ec);
				m_disconnectHandler = nullptr;
				m_readHandler = nullptr;
			}

		}

		void StartRead() {
			Log(LogLevel::Debug, "impl::ClientSocket::StartRead()");
			m_socket.async_read_some(m_readBuffer.prepare(ReadChunkSize),
									[this, self{Ptr()}](YAARC_ERROR_CODE ec, size_t read) {
										HandleRead(ec, read);
									});
		}

		void HandleRead(YAARC_ERROR_CODE ec, size_t read) {
			Log(LogLevel::Debug, "impl::ClientSocket::HandleRead()");
			if (!m_readHandler) {
				// this socket is dead now, since we don't have a read handler anymore
				return;
			}
			if (ec) {
				OnDisconnected(ec);
				return;
			}
			m_readBuffer.commit(read);
			size_t consumed;
			Value value;
			while (ParseRESP(m_readBuffer.data().data(), m_readBuffer.data().size(), consumed, value)) {
				m_readBuffer.consume(consumed);
				if (m_readHandler) {
					m_readHandler(value);
				} else {
					return;
				}
			}

			StartRead();
		}
	};
}

#endif //YAARC_CLIENTSOCKET_HPP
