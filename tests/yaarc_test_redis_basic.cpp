#include "yaarc_test_helper.hpp"
#include <yaarc.hpp>

int main() {
	yaarc::Config config;
	config.Host = "localhost";
	config.Password = "";
	asio::io_context io;
	yaarc::Yaarc yaarc(io, config);

	yaarc.Get("test", [](std::error_code ec, yaarc::Value value) {
		std::cout << fmt::format("Got EC: {}, value: {}", ec.value(), value) << std::endl;
	});

	io.run();
}

