# yaarc - Yet Another Asio Redis Client
Simple redis client with goal of resiliance and ease of use.

yaarc currently only supports commands, subscribing and other "advanced" features aren't supported at the moment

## Why another?
Existing clients were lacking on the front of error handling abstraction.

yaarc will handle connection errors gracefully and simply retry up to a configurable limit

## Usage
TODO
```c++
#include <yaarc.hpp>

void example() {
	yaarc::Config config;
	config.Host = "localhost";
	// port is 6379 by default
	// config.Port = 6379
	config.Password = "my password";
	// you can also set timeouts/max retries:
	// config.ConnectTimeout = std::chrono::milliseconds(10000);
	// config.ReadTimeout = std::chrono::milliseconds(10000);
	// config.CommandRetries = 10;
	auto client = yaarc::Yaarc(config);
	client->set("some-key", "some-value", [](yaarc::Value result, std::error_code error){
		
	});
	
	client->get("some-other-key", [](yaarc::Value value, std::error_code error) {
		if (error) {
			std::cout << "Get failed with code " << error.code() << std::endl;
			return;
		}
        if (value.is_null()) {
            std::cout << "some-other-key is null" << std::endl;
        } else if (value.is_error()) {
            std::cout << "some-other-key is a string with value '" << value.as_string_view() << "'" << std::endl;
        } else if (value.is_string()) {
            std::cout << "some-other-key is a string with value '" << value.as_string_view() << "'" << std::endl;
        } else if (value.is_int()) {
            std::cout << "some-other-key is an int with value '" << value.as_int() << "'" << std::endl;
        }
	});
}
```