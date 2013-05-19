awake
=====

Wake-on-LAN with Boost.Asio


Getting started
---------------

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
