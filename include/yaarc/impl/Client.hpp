#ifndef YAARC_IMPL_CLIENT_HPP
#define YAARC_IMPL_CLIENT_HPP

#include "IncludeAsio.hpp"
#include "../Config.hpp"
#include "../Value.hpp"
#include "../LogLevel.hpp"
#include "../ValueParser.hpp"
#include "Command.hpp"
#include "ClientSocket.hpp"
#include <deque>
#include <functional>
#include <memory>
#include <utility>

namespace yaarc::impl {
	class Client : public std::enable_shared_from_this<Client> {
		yaarc::Config m_config;
		YAARC_ASIO::ip::tcp::resolver m_resolver;

		std::deque<Command> m_pendingCommands;
		std::deque<Command> m_sentCommands;
		std::function<void(LogLevel, std::string)> m_logger;
		YAARC_ASIO::steady_timer m_reconnectTimer;
		bool m_isConnected;
		std::vector<uint8_t> m_authBuffer;
		std::shared_ptr<ClientSocket<decltype(m_resolver.get_executor())>> m_socket;
		bool m_isWriting;
	public:
		Client() = delete;

		explicit Client(YAARC_ASIO::io_context& io, Config config) :
				m_config(std::move(config)),
				m_resolver(YAARC_ASIO::make_strand(io)),
				m_reconnectTimer(m_resolver.get_executor()),
				m_isConnected(false),
				m_isWriting(false) {

		}

		std::shared_ptr<Client> Ptr() {
			return shared_from_this();
		}

		void SetLogger(std::function<void(LogLevel, std::string)> handler) {
			m_logger = std::move(handler);
			if (m_socket) {
				m_socket->SetLogger(m_logger);
			}
		}

		void Connect() {
			if (m_isConnected) {
				throw std::runtime_error("Already connected.");
			}
			Resolve();
		}

		void Cmd(const Value& command, std::function<void(std::error_code, Value)> callback) {
			if (!command.IsArray()) {
				throw std::runtime_error("Redis command needs to be an array");
			}
			std::vector<uint8_t> encodedCommand;
			EncodeRESPBulkString(command, encodedCommand);
			YAARC_ASIO::dispatch(m_resolver.get_executor(),
						   [this, self{Ptr()}, encodedCommand{std::move(encodedCommand)}, callback{
								   std::move(callback)}]() {
							   m_pendingCommands.emplace_back(
									   std::move(encodedCommand),
									   std::move(callback)
							   );
							   // if we're not writing, but connected, our buffer is empty
							   // so we need to kickstart writing again
							   if (m_isConnected && !m_isWriting) {
								   StartWrite();
							   }
						   });
		};

		/// Stops the client, all pending commands are cancelled
		template<class Callback>
		void Stop(Callback handler) {
			YAARC_ASIO::dispatch(m_resolver.get_executor(), [this, handler, self{Ptr()}]() {

				m_resolver.cancel();
				m_reconnectTimer.cancel();

				for (auto& cmd : m_sentCommands) {
					cmd.Invoke(YAARC_ERROR_CODE(YAARC_ASIO::error::operation_aborted), Value());
				}
				m_sentCommands.clear();

				for (auto& cmd : m_pendingCommands) {
					cmd.Invoke(YAARC_ERROR_CODE(YAARC_ASIO::error::operation_aborted), Value());
				}
				m_pendingCommands.clear();
				// if we have a socket, dispatch handler to it
				if (m_socket) {
					m_socket->Stop(handler);
					m_socket.reset();
				} else {
					handler(YAARC_ERROR_CODE());
				}
			});
		}

	private:
		void ScheduleReconnect() {
			m_reconnectTimer.expires_after(m_config.ReconnectInterval);

			Log(LogLevel::Info, fmt::format("Attempting to reconnect in {}ms",
											m_config.ReconnectInterval.count()));
			m_reconnectTimer.async_wait([this, self{Ptr()}](std::error_code ec) {
				if (ec) {
					return;
				}
				Resolve();
			});
		}

		void Log(LogLevel level, std::string error) {
			if (m_logger) {
				m_logger(level, std::move(error));
			}
		}

		void Resolve() {
			m_resolver.async_resolve(m_config.Host, std::to_string(m_config.Port), [this, self{Ptr()}](const std::error_code& ec,
																		YAARC_ASIO::ip::tcp::resolver::results_type results) {
				if (ec) {
					if (ec.value() != YAARC_ASIO::error::operation_aborted) {
						Log(LogLevel::Error, fmt::format("Failed to resolve '{}' during connect due to error {} (#{})",
														 m_config.Host, ec.message(), ec.value()));
						ScheduleReconnect();
						return;
					}
				}
				if (results.empty()) {
					Log(LogLevel::Error,
						fmt::format("Failed to resolve '{}' during connect - no results, hostname unknown",
									m_config.Host));
					ScheduleReconnect();
					return;
				}
				Log(LogLevel::Debug,
					fmt::format("Resolved {}, resulted in {} entries. Trying to connect now", m_config.Host,
								results.size()));
				TryConnect(results);
			});
		}

		void TryConnect(YAARC_ASIO::ip::tcp::resolver::results_type resolved, size_t offset = 0) {
			if (offset >= resolved.size()) {
				throw std::runtime_error(
						fmt::format("Bigger resolve offset passed to TryConnect than exists (offset {} >= size {})",
									offset, resolved.size()));
			}
			YAARC_ASIO::dispatch(m_resolver.get_executor(), [this, self{Ptr()}, resolved{std::move(resolved)}, offset]() {
				if (m_socket) {
					m_socket->Stop([](YAARC_ERROR_CODE) {});
					Log(LogLevel::Debug, fmt::format("Socket was open while trying to connect, closing"));
					m_socket.reset();
				}
				m_socket = std::make_shared<ClientSocket<decltype(m_resolver.get_executor())>>(
						m_resolver.get_executor(),
						[this, self{Ptr()}](Value value) {
							HandleRead(std::move(value));
						},
						[this, self{Ptr()}](YAARC_ERROR_CODE ec) {
							OnDisconnected(ec);
						}
				);
				m_socket->SetLogger(m_logger);
				auto it = resolved.begin();
				for (size_t i = 0; i < offset; i++) {
					it++;
				}
				auto endpoint = it->endpoint();
				endpoint.port(m_config.Port);
				Log(LogLevel::Info,
					fmt::format("Trying to connect to {}:{}", endpoint.address().to_string(), endpoint.port()));
				m_socket->TryConnect(endpoint, [this, self, endpoint, resolved, offset](std::error_code ec) {
					if (ec) {
						if (ec.value() == YAARC_ASIO::error::operation_aborted) {
							return;
						}

						Log(LogLevel::Warning,
							fmt::format("Failed to connect to {}:{}", endpoint.address().to_string(), endpoint.port()));
						if (offset + 1 >= resolved.size()) {
							ScheduleReconnect();
							return;
						}
						TryConnect(resolved, offset + 1);
						return;
					}
					OnConnected();
				});
			});
		}

		void OnConnected() {
			Log(LogLevel::Debug, "impl::Client::OnConnected");
			if (m_config.Password.empty()) {
				m_isConnected = true;
				YAARC_ASIO::post(m_resolver.get_executor(), [this, self{Ptr()}]() {
					StartWrite();
				});
			} else {
				m_authBuffer.clear();
				// for the auth command, we partially re-use the send logic
				// we write a bulk string to the socket an then use the existing send/write handlers
				EncodeRESPBulkString(Value(std::vector<Value>{
						Value("AUTH"),
						Value(m_config.Password)
				}), m_authBuffer);
				m_sentCommands.emplace_front(Command{
						m_authBuffer, [this, self{Ptr()}](std::error_code ec, Value res) {
							if (ec) {
								// ec is handled via the disconnect handler
								return;
							}
							if (res.IsError()) {
								Log(LogLevel::Error, fmt::format("Failed to AUTH against server: {}", res.AsError()));
								m_socket->Stop([](YAARC_ERROR_CODE) {});
								m_socket.reset();
								ScheduleReconnect();
							} else {
								m_isConnected = true;
								YAARC_ASIO::post(m_resolver.get_executor(), [this, self{Ptr()}]() {
									StartWrite();
								});
							}
						}, std::numeric_limits<size_t>::max()
				});
				m_socket->StartWrite(m_sentCommands.begin(), ++m_sentCommands.begin(), [](YAARC_ERROR_CODE) {
					// nothing to do here, the auth response comes through the read handler
				});
			}
		}

		void StartWrite() {
			if (m_isWriting) {
				throw std::runtime_error("Tried to StartWrite() without a socket set");
			}
			if (!m_socket) {
				throw std::runtime_error("Tried to StartWrite() without a socket set");
			}
			if (m_pendingCommands.empty()) {
				return;
			}
			m_isWriting = true;
			Log(LogLevel::Debug, "impl::Client::StartWrite");

			// TODO: currently we just shove all pending commands into the write buffer at once
			// this *should* be fine in most cases?
			m_socket->StartWrite(m_pendingCommands.begin(), m_pendingCommands.end(), [this, self{Ptr()}](YAARC_ERROR_CODE ec) {
				// Restart write right away once we have written everything
				m_isWriting = false;
				if (!ec) {
					YAARC_ASIO::post(m_resolver.get_executor(), [this, self{Ptr()}]() {
						if (m_socket) {
							StartWrite();
						}
					});
				}
			});
			for (auto cmd : m_pendingCommands) {
				m_sentCommands.push_back(std::move(cmd));
			}
			m_pendingCommands.clear();
		}

		void HandleRead(Value value) {
			if (m_sentCommands.empty()) {
				throw std::runtime_error("Called read handler with empty m_sentCommands");
			}
			m_sentCommands.front().Invoke(std::error_code(), value);
			m_sentCommands.pop_front();
		}

		void OnDisconnected(std::error_code ec) {
			// re-queue any commands that were still pending
			while (!m_sentCommands.empty()) {
				auto cmd = std::move(m_sentCommands.back());
				m_sentCommands.pop_back();
				cmd.FailCount++;
				if (cmd.FailCount < m_config.CommandRetries) {
					m_pendingCommands.emplace_front(std::move(cmd));
				} else {
					cmd.Invoke(ec, Value());
				}
			}
			m_sentCommands.clear();

			m_isConnected = false;
			ScheduleReconnect();
		}
	};
}
#endif //YAARC_IMPL_CLIENT_HPP
