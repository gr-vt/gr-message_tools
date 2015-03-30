# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND message_tools_sources
    message_file_sink_impl.cc
    message_strobe_source_impl.cc
    msg_vector_strobe_impl.cc
    msg_vector_sink_impl.cc
)

add_library(gnuradio-message_tools SHARED ${message_tools_sources})
target_link_libraries(gnuradio-message_tools ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-message_tools PROPERTIES DEFINE_SYMBOL "gnuradio_message_tools_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-message_tools PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-message_tools
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_message_tools_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_message_tools.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_message_tools.cc
)

add_executable(test-message_tools ${test_message_tools_sources})

target_link_libraries(
  test-message_tools
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-message_tools
)

GR_ADD_TEST(test_message_tools test-message_tools)
