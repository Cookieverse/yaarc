#ifndef INFERNA_TUNNEL_REDISCLIENT_HPP
#define INFERNA_TUNNEL_REDISCLIENT_HPP
namespace yaarc {
	class RedisClient {
	private:
		std::shared_ptr<impl::Client> m_impl;
	public:
		RedisClient(std::string host, ushort port = 6379, std::string password = "") : m_impl(std::move(host), port, std::move(password)) {

		}
	};
}

#endif //INFERNA_TUNNEL_REDISCLIENT_HPP
