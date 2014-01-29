INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_MESSAGE_FILE message_file)

FIND_PATH(
    MESSAGE_FILE_INCLUDE_DIRS
    NAMES message_file/api.h
    HINTS $ENV{MESSAGE_FILE_DIR}/include
        ${PC_MESSAGE_FILE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    MESSAGE_FILE_LIBRARIES
    NAMES gnuradio-message_file
    HINTS $ENV{MESSAGE_FILE_DIR}/lib
        ${PC_MESSAGE_FILE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MESSAGE_FILE DEFAULT_MSG MESSAGE_FILE_LIBRARIES MESSAGE_FILE_INCLUDE_DIRS)
MARK_AS_ADVANCED(MESSAGE_FILE_LIBRARIES MESSAGE_FILE_INCLUDE_DIRS)

