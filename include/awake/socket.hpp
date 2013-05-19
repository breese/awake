#ifndef AWAKE_SOCKET_HPP
#define AWAKE_SOCKET_HPP

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

#include <awake/basic_socket.hpp>

namespace awake
{
namespace udp
{

typedef basic_socket<boost::asio::ip::udp> socket;

} // namespace udp
} // namespace awake

#endif // AWAKE_SOCKET_HPP
