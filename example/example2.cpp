///////////////////////////////////////////////////////////////////////////////
//
// http://github.com/breese/awake
//
// Copyright (C) 2013 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstdio> // std::sscanf
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/asio.hpp>
#include <awake/socket.hpp>

class awaker
{
public:
    awaker(boost::asio::io_service& io)
        : socket(io),
          endpoint(boost::asio::ip::address_v4::broadcast(), 9)
    {
    }

    void async_awake(const char *input)
    {
        boost::array<unsigned int, 6> address;
        std::sscanf(input, "%02x:%02x:%02x:%02x:%02x:%02x",
                    &address[0], &address[1], &address[2], &address[3], &address[4], &address[5]);
        socket.async_awake(address,
                           boost::protect(boost::bind(&awaker::process_awake,
                                                      this,
                                                      boost::asio::placeholders::error)));
    }

    void process_awake(const boost::system::error_code& error)
    {
        std::cout << __FUNCTION__ << "(" << error << ", " << socket.local_endpoint() << ")" << std::endl;
    }

    awake::udp::socket socket;
    awake::udp::socket::endpoint_type endpoint;
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <mac>" << std::endl;
        return 1;
    }
    boost::asio::io_service io;
    awaker socket(io);
    socket.async_awake(argv[1]);
    io.run();

    return 0;
}
