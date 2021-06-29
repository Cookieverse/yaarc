#ifndef INFERNA_TUNNEL_INCLUDEASIO_HPP
#define INFERNA_TUNNEL_INCLUDEASIO_HPP

#ifdef YAARC_USE_BOOST_ASIO
#include <boost/asio.hpp>
#define YAARC_ASIO boost::asio
#define YAARC_ERROR_CODE boost::system::error_code
#else
#include <asio.hpp>
#define YAARC_ASIO asio
#define YAARC_ERROR_CODE asio::error_code
#endif
#endif //INFERNA_TUNNEL_INCLUDEASIO_HPP
