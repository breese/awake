#ifndef AWARE_BASIC_SOCKET_HPP
#define AWARE_BASIC_SOCKET_HPP

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

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread_time.hpp>

namespace awake
{

template <typename Protocol>
class basic_socket
{
    // Constants
    enum
    {
        // Magic packet sizes
        header_size = 6,
        mac_size = 6,
        mac_count = 16,
        // Send parameters
        burst_count = 6,
        retry_count = 3,
        retry_delay = 300 // milliseconds
    };

    typedef boost::asio::ip::udp::socket socket_type;

public:
    typedef Protocol protocol_type;
    typedef unsigned char char_type;
    typedef socket_type::endpoint_type endpoint_type;

private:
    typedef char_type magic_packet_type[header_size + mac_count * mac_size];

    class task
    {
    public:
        template <typename InputIterator>
        task(boost::asio::io_service& io,
             InputIterator first,
             InputIterator last);

        bool next_burst();
        bool next_retry();
        bool next_deadline(boost::system::error_code& error);
        template <typename Handler>
        void async_wait(BOOST_ASIO_MOVE_ARG(Handler) handler);

        const magic_packet_type& get_magic_packet() const;

    private:
        unsigned int bursts;
        unsigned int retries;
        magic_packet_type magic_packet;
        boost::asio::deadline_timer timer;
        boost::posix_time::ptime deadline;
    };

public:
    //! @brief Construct a socket.
    //!
    //! The endpoint is port 9 on the IPv4 broadcast address.
    //!
    //! @param io The io_service that is serving this socket.
    basic_socket(boost::asio::io_service& io);

    //! @brief Construct a socket.
    //!
    //! @param io The io_service that is serving this socket.
    //! @param endpoint The endpoint to send the UDP magic packets to.
    //! Different devices listens for magic packets on different UDP ports,
    //! none of which are registered with IANA. Typically port 7, 9, or 2304
    //! is used.
    basic_socket(boost::asio::io_service& io,
                 endpoint_type endpoint);

    //! @brief Send wake-up packets.
    //!
    //! This function sends magic Wake-on-LAN packets to a sleeping device via UDP broadcast.
    //!
    //! @param mac The Media Access Control (MAC) address as an array of six octets.
    //! SecureOn is not supported.
    //! @param handler The handler to be called when the wake-up packets have been sent.
    //! The function signature of the handler must be:
    //! @code void handler(
    //!   const boost::system::error_code& error // Result of operation
    //! }; @endcode
    template <typename SinglePassRange, typename Handler>
    void async_awake(SinglePassRange mac,
                     BOOST_ASIO_MOVE_ARG(Handler) handler);

    boost::asio::io_service& get_io_service();
    endpoint_type local_endpoint() const;
    endpoint_type local_endpoint(boost::system::error_code&) const;

private:
    template <typename Handler>
    void async_send_burst(boost::shared_ptr<task> current_task,
                          BOOST_ASIO_MOVE_ARG(Handler) handler);

    template <typename Handler>
    void process_burst(const boost::system::error_code& error,
                       std::size_t bytes_transferred,
                       boost::shared_ptr<task> current_task,
                       Handler handler);

    template <typename Handler>
    void async_retry(boost::shared_ptr<task> current_task,
                     BOOST_ASIO_MOVE_ARG(Handler) handler);

private:
    socket_type socket;
    endpoint_type endpoint;
};

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

template <typename Protocol>
basic_socket<Protocol>::basic_socket(boost::asio::io_service& io)
    : socket(io),
      endpoint(boost::asio::ip::address_v4::broadcast(), 9)
{
    socket.open(endpoint.protocol());
    socket.set_option(boost::asio::socket_base::broadcast(true));
}

template <typename Protocol>
basic_socket<Protocol>::basic_socket(boost::asio::io_service& io,
                                     endpoint_type endpoint)
    : socket(io),
      endpoint(endpoint)
{
    socket.open(endpoint.protocol());
    socket.set_option(boost::asio::socket_base::broadcast(true));
}

template <typename Protocol>
template <typename SinglePassRange, typename Handler>
void basic_socket<Protocol>::async_awake(SinglePassRange mac,
                                         BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    // Create magic packet and pass ownership
    boost::shared_ptr<task> current_task(new task(socket.get_io_service(),
                                                  boost::begin(mac),
                                                  boost::end(mac)));
    async_send_burst(current_task, handler);
}

template <typename Protocol>
boost::asio::io_service& basic_socket<Protocol>::get_io_service()
{
    return socket.get_io_service();
}

template <typename Protocol>
typename basic_socket<Protocol>::endpoint_type
basic_socket<Protocol>::local_endpoint() const
{
    return socket.local_endpoint();
}

template <typename Protocol>
typename basic_socket<Protocol>::endpoint_type
basic_socket<Protocol>::local_endpoint(boost::system::error_code& error) const
{
    return socket.local_endpoint(error);
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::async_send_burst(boost::shared_ptr<task> current_task,
                                              BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    socket.async_send_to(boost::asio::buffer(current_task->get_magic_packet()),
                         endpoint,
                         boost::bind(&basic_socket::process_burst<Handler>,
                                     this,
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred,
                                     current_task,
                                     handler));
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::process_burst(const boost::system::error_code& error,
                                           std::size_t bytes_transferred,
                                           boost::shared_ptr<task> current_task,
                                           Handler handler)
{
    if (error)
    {
        handler(error);
    }
    else
    {
        // FIXME: Check bytes_transferred ?

        if (current_task->next_burst())
        {
            async_send_burst(current_task, handler);
        }
        else if (current_task->next_retry())
        {
            async_retry(current_task, handler);
        }
        else
        {
            // We are (hopefully) done
            handler(error);
        }
    }
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::async_retry(boost::shared_ptr<task> current_task,
                                         BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    boost::system::error_code error;
    if (current_task->next_deadline(error))
    {
        current_task->async_wait(boost::bind(&basic_socket::async_send_burst<Handler>,
                                             this,
                                             current_task,
                                             handler));
    }
    else
    {
        handler(error);
    }
}

template <typename Protocol>
template <typename InputIterator>
basic_socket<Protocol>::task::task(boost::asio::io_service& io,
                                   InputIterator first,
                                   InputIterator last)
    : bursts(burst_count),
      retries(retry_count),
      timer(io)
{
    assert(std::distance(first, last) == mac_size);
    deadline = boost::get_system_time() + boost::posix_time::milliseconds(retry_delay);
    std::fill_n(magic_packet, header_size, 0xFF);
    for (unsigned int i = 0; i < mac_count; ++i)
    {
        std::copy(first, last, &magic_packet[header_size + i * mac_size]);
    }
}

template <typename Protocol>
bool basic_socket<Protocol>::task::next_burst()
{
    --bursts;
    return (bursts > 0);
}

template <typename Protocol>
bool basic_socket<Protocol>::task::next_retry()
{
    bursts = burst_count;
    --retries;
    return (retries > 0);
}

template <typename Protocol>
bool basic_socket<Protocol>::task::next_deadline(boost::system::error_code& error)
{
    timer.expires_at(deadline, error);
    deadline += boost::posix_time::milliseconds(retry_delay);
    return !error;
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::task::async_wait(BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    timer.async_wait(handler);
}

template <typename Protocol>
const typename basic_socket<Protocol>::magic_packet_type&
basic_socket<Protocol>::task::get_magic_packet() const
{
    return magic_packet;
}

} // namespace awake


#endif // AWARE_BASIC_SOCKET_HPP
