cmake_minimum_required(VERSION 3.2)
include(CPack)
include(CheckFunctionExists)

project(mbusd VERSION 0.2.4)

#TODO  ISC_Posix,  prog_libtool
# single-configuration generator setup
SET(BASIC_C_FLAGS "-W -pedantic -fno-builtin-log -Wall")
SET(CMAKE_C_FLAGS_RELEASE   "${BASIC_C_FLAGS} -O2")
SET(CMAKE_C_FLAGS_DEBUG     "${BASIC_C_FLAGS} -g")

add_definitions(-DPACKAGE="${PROJECT_NAME}" -DVERSION="${PROJECT_VERSION}")

# user options
option (TRXCTL "Support RS-232 to RS-485 converter data direction control" ON)
if(TRXCTL)
    add_definitions(-DTRXCTL)
endif()
option (LOG "enabling logging facility" ON)
if(LOG)
    add_definitions(-DLOG)
endif()
option(DEBUG_LOG "extra debug log info" ON)
if(DEBUG_LOG)
    add_definitions(-DDEBUG)
endif()

## check for and pass preprocessor flags
check_function_exists(cfmakeraw  HAVE_CFMAKERAW)
if(HAVE_CFMAKERAW)
    add_definitions(-DHAVE_CFMAKERAW)
endif()
check_function_exists(cfsetspeed  HAVE_CFSETSPEED)
check_function_exists(cfsetispeed HAVE_CFSETISPEED)
if(HAVE_CFSETSPEED AND HAVE_CFSETISPEED)
    add_definitions(-DHAVE_CFSETSPEED)
endif()
check_function_exists(time HAVE_TIME)
check_function_exists(localtime HAVE_LOCALTIME)
if(HAVE_TIME AND HAVE_LOCALTIME)
    message("passing HRDATE to compiler space")
    add_definitions(-DHRDATE)
endif()

include(CheckLibraryExists)
find_library(LIB_UTIL NAMES libutil util)
check_library_exists(util tty_get_name LIB_UTIL HAVE_TTY_GET_NAME)
check_library_exists(util uu_lock LIB_UTIL HAVE_UU_LOCK)
if(LIB_UTIL AND HAVE_TTY_GET_NAME AND HAVE_UU_LOCK)
    message("passing HAVE_LIBUTIL to compiler space")
    add_definitions(-DHAVE_LIBUTIL)
endif()


set(mbusd_SOURCES
    src/main.c
    src/tty.c
    src/log.c
    src/cfg.c
    src/conn.c
    src/queue.c
    src/modbus.c
    src/crc16.c
    src/state.c
    src/sig.c
    src/sock.c
)
add_executable(mbusd ${mbusd_SOURCES})