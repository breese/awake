awake
=====

Wake-on-LAN with [Boost.Asio](http://www.boost.org/doc/libs/release/libs/asio/).

Introduction
------------

The awake module provides a socket that broadcasts UDP magic packets to wake up
a sleeping device. Several devices can be awoken concurrently. To increase the
likelihood of success, several bursts of UDP packets are send within a second.

An example of the use of awake:

```c++
#include <boost/asio/io_service.hpp>
#include <aware/socket.hpp>

void AwakeHandler(const boost::system::error_code& error)
{
  // The magic packets have been send, or an error occurred along the way.
  // Please notice that we do not know if the device has been awoken, and
  // if so, whether or not it is ready for use.
}

int main()
{
  boost::asio::io_service io;
  awake::udp::socket wakeup(io);
  awake::udp::socket::mac_address_type address; // Must be initialized with the MAC address of the sleeping device
  wakeup.async_awake(address, AwakeHandler); // Send the Wake-on-LAN packets asynchronously
  io.run();
  return 0;
}
```

Building awake
--------------

1. Make sure [CMake](http://cmake.org/) is installed.

2. Checkout the repository.

3. Build the example

   ```bash
   cmake .
   make
   ```
4. Run the example (replace 11:22:33:44:55:66 with the MAC address of the sleeping device)

   ```bash
   bin/exawake 11:22:33:44:55:66
   ```
