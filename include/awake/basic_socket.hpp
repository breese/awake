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

#include <algorithm>
#include <vector>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread_time.hpp>

namespace awake
{

template <typename Protocol>
class basic_socket
{
    enum
    {
        header_size = 6,
        mac_size = 6,
        mac_count = 16,
        retry_delay = 300 // milliseconds
    };

public:
    typedef Protocol protocol_type;
    typedef boost::array<unsigned char, mac_size> mac_address_type;

private:
    typedef boost::array<unsigned char, header_size + mac_count * mac_size> magic_packet_type;

    struct task
    {
        enum
        {
            burst_count = 6,
            retry_count = 2
        };

        task(boost::asio::io_service& io,
             const mac_address_type& address)
            : bursts(burst_count),
              retries(retry_count),
              deadline(io)
        {
            std::fill_n(magic_packet.begin(), header_size, 0xFF);
            for (unsigned int i = 0; i < mac_count; ++i)
            {
                std::copy(address.begin(), address.end(), &magic_packet[header_size + i * mac_size]);
            }
        }

        unsigned int bursts;
        unsigned int retries;
        magic_packet_type magic_packet;
        boost::asio::deadline_timer deadline;
    };

public:
    basic_socket(boost::asio::io_service& io);

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
    template <typename Handler>
    void async_awake(const mac_address_type mac,
                     BOOST_ASIO_MOVE_ARG(Handler) handler);

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
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint endpoint;
    boost::posix_time::ptime next_retry;
};

//-----------------------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------------------

template <typename Protocol>
basic_socket<Protocol>::basic_socket(boost::asio::io_service& io)
    : socket(io),
      endpoint(boost::asio::ip::address_v4::broadcast(), 9) // FIXME: Which port? 7 or 9?
{
    socket.open(boost::asio::ip::udp::v4());
    socket.set_option(boost::asio::socket_base::broadcast(true));
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::async_awake(const mac_address_type mac,
                                         BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    // Create magic packet and pass ownership
    next_retry = boost::get_system_time() + boost::posix_time::milliseconds(retry_delay);
    boost::shared_ptr<task> current_task(new task(socket.get_io_service(), mac));
    async_send_burst(current_task, handler);
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::async_send_burst(boost::shared_ptr<task> current_task,
                                              BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    --(current_task->bursts);
    socket.async_send_to(boost::asio::buffer(current_task->magic_packet),
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

        if (current_task->bursts > 0)
        {
            async_send_burst(current_task, handler);
        }
        else
        {
            if (current_task->retries > 0)
            {
                current_task->bursts = task::burst_count;
                async_retry(current_task, handler);
            }
            else
            {
                // We are (hopefully) done
                handler(error);
            }
        }
    }
}

template <typename Protocol>
template <typename Handler>
void basic_socket<Protocol>::async_retry(boost::shared_ptr<task> current_task,
                                         BOOST_ASIO_MOVE_ARG(Handler) handler)
{
    --(current_task->retries);
    boost::system::error_code error;
    current_task->deadline.expires_at(next_retry, error);
    if (error)
    {
        handler(error);
    }
    else
    {
        next_retry += boost::posix_time::milliseconds(retry_delay);
        current_task->deadline.async_wait(boost::bind(&basic_socket::async_send_burst<Handler>,
                                                      this,
                                                      current_task,
                                                      handler));
    }
}

} // namespace awake


#endif // AWARE_BASIC_SOCKET_HPP
