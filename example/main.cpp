#include <iostream>
#include <cstdio> // std::sscanf
#include <boost/asio/io_service.hpp>
#include <awake/socket.hpp>

void done(const boost::system::error_code& error)
{
    std::cout << __FUNCTION__ << "(" << error << ")" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <mac>" << std::endl;
        return 1;
    }
    boost::asio::io_service io;
    awake::udp::socket awaker(io);
    awake::udp::socket::mac_address_type address;
    boost::array<unsigned int, awake::udp::socket::mac_address_type::static_size> input;
    if (std::sscanf(argv[1], "%02x:%02x:%02x:%02x:%02x:%02x",
                    &input[0], &input[1], &input[2], &input[3], &input[4], &input[5]) == address.size())
    {
        address[0] = input[0] & 0xFF;
        address[1] = input[1] & 0xFF;
        address[2] = input[2] & 0xFF;
        address[3] = input[3] & 0xFF;
        address[4] = input[4] & 0xFF;
        address[5] = input[5] & 0xFF;
        awaker.async_awake(address, done);
        io.run();
    }
    return 0;
}
