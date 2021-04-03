#ifndef YAARC_CONFIG_HPP
#define YAARC_CONFIG_HPP

#include <string>
#include <cstdint>
#include <chrono>


namespace yaarc {
	struct Config {
		/// What host to connect to, ipv4/6 or dns
		std::string Host;
		/// What port to connect to, defaults to standard redis port
		uint16_t Port = 6379;
		/// If a password is specified, new connections will be AUTH-ed with this password
		std::string Password;
		/// How long to wait for a connection attempt to time out
		/// if not specified (zero) this defaults to the system timeout
		std::chrono::milliseconds ConnectTimeout; // TODO
		/// How long to wait for redis to respond
		/// if not specified it waits forever
		std::chrono::milliseconds ReadTimeout; // TODO
		// If connecting to the redis server fails, how often to retry
		std::chrono::milliseconds ReconnectInterval = std::chrono::milliseconds(500);
		/// If command execution fails due to connection reasons (not redis errors, includes failed re-connects)
		/// The command will be retries CommandRetries times
		size_t CommandRetries;
		/// While sending if there's multiple commands in the queue they will be copied into a buffer and sent together
		/// This increases throughput
		/// This value is a soft limit, if the buffer exceeds the specified size, it will be sent
		/// otherwise it will copy all commands off the queue info the buffer
		size_t WriteBatchingTarget = 1024 * 1024;
	};
}
#endif //YAARC_CONFIG_HPP
