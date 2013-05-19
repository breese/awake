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

###############################################################################
# Boost package
###############################################################################

# FIXME: Probably ok to use older versions
find_package(Boost 1.49.0 COMPONENTS thread system)
if (NOT ${Boost_FOUND})
  message(FATAL_ERROR "Boost not found (or too old)")
endif()
include_directories(BEFORE ${Boost_INCLUDE_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})

###############################################################################
# awake package
###############################################################################

#set(AWAKE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(BEFORE include)
#link_directories(${AWAKE_LIB_DIR})
#set(LIBRARY_OUTPUT_PATH lib)
#set(EXTRA_LIBS -lawake ${EXTRA_LIBS})

#add_library(awake STATIC
#  src/socket.cpp
#)

###############################################################################
# Example
###############################################################################

add_executable(exawake
  example/main.cpp
)

add_dependencies(exawake awake)
set_target_properties(exawake PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(exawake ${EXTRA_LIBS})

