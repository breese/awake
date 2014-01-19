###############################################################################
#
# Copyright (C) 2013 Bjorn Reese <breese@users.sourceforge.net>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
# MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
# CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
#
###############################################################################

# Debug build
#   cmake -DCMAKE_BUILD_TYPE=Debug .

###############################################################################

cmake_minimum_required(VERSION 2.8)
project(awake)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(AWAKE_BUILD_DIR ${CMAKE_BINARY_DIR}/stage)
set(LIBRARY_OUTPUT_PATH ${AWAKE_BUILD_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${AWAKE_BUILD_DIR}/bin)

###############################################################################
# Boost package
###############################################################################

# FIXME: Probably ok to use older versions
find_package(Boost 1.49.0 COMPONENTS chrono system)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "Boost not found (or too old)")
endif()
include_directories(BEFORE ${Boost_INCLUDE_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})

###############################################################################
# awake package
###############################################################################

include_directories(BEFORE include)

###############################################################################
# Example
###############################################################################

add_executable(example1
  example/example1.cpp
)

add_dependencies(example1 awake)
target_link_libraries(example1 ${EXTRA_LIBS})

add_executable(example2
  example/example2.cpp
)

add_dependencies(example2 awake)
target_link_libraries(example2 ${EXTRA_LIBS})
