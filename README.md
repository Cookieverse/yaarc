# yaarc - Yet Another Asio Redis Client
Simple redis client with goal of resiliance and ease of use
yaarc currently only supports commands, subscribing and other "advanced" features aren't supported at the moment

## Why another?
Existing clients were lacking on the front of error handling abstraction
yaarc will handle connection errors gracefully and simply retry up to a configurable limit
