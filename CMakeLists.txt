cmake_minimum_required(VERSION 3.2)

project(mbusd VERSION 0.3.1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/extern_GPL)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(FindUnixCommands)
include(FindSystemd)

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
    message(STATUS "Passing HRDATE to compiler space")
    add_definitions(-DHRDATE)
endif()

find_library(LIB_UTIL NAMES libutil util)
check_library_exists(util tty_get_name LIB_UTIL HAVE_TTY_GET_NAME)
check_library_exists(util uu_lock LIB_UTIL HAVE_UU_LOCK)
if(LIB_UTIL AND HAVE_TTY_GET_NAME AND HAVE_UU_LOCK)
    message(STATUS "Passing HAVE_LIBUTIL to compiler space")
    add_definitions(-DHAVE_LIBUTIL)
endif()

# add the main application
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
install(TARGETS mbusd DESTINATION bin)

# aggregate the man page template
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/mbusd.8.in mbusd.8)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mbusd.8 DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/man/man8)

# install example configuration file
install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/conf/mbusd.conf.example
        DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}
)

if(SYSTEMD_SERVICES_INSTALL_DIR)
    message(STATUS "Systemd service file will be installed to ${SYSTEMD_SERVICES_INSTALL_DIR}")
    # aggregate mbusd@.service from its template
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/systemd-units/mbusd@.service.in mbusd@.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mbusd@.service DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR})
endif()

# uninstall target
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)
add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

# integration tests target
enable_testing()
add_test(
	NAME itests
	COMMAND ${BASH} -c "cd ${CMAKE_CURRENT_SOURCE_DIR}/tests && ./run_itests.sh $<TARGET_FILE:mbusd>"
)

## Please find Packaging stuff following
#@source http://xit0.org/2013/04/cmake-use-git-branch-and-commit-details-in-project/
# Get the current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Get the latest abbreviated commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

## issue the package creation with $make package
# which infrastructure do we want
set(CPACK_GENERATOR "DEB")
find_program(rpmBuilder rpmbuild)
if(rpmBuilder)
    # @see https://schneide.wordpress.com/2013/02/11/build-a-rpm-package-using-cmake/
    # @see http://www.g-loaded.eu/2006/04/05/how-to-build-rpm-packages-on-fedora/
    set(CPACK_GENERATOR "RPM")
endif()

set(CPACK_PACKAGE_NAME "mbusd")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Victor Antonovich") #required
set(CPACK_PACKAGE_CONTACT           "Victor Antonovich <v.antonovich@gmail.com>")
set(CPACK_PACKAGE_VERSION   ${PROJECT_VERSION}-${GIT_BRANCH}-g${GIT_COMMIT_HASH})
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}-v${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modbus TCP to Modbus RTU gateway")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_STRIP_FILES TRUE)
#the include must be after all the cpack concerning set's
include(CPack)

message(STATUS "Install prefix      ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Install bindir:     ${CMAKE_INSTALL_FULL_BINDIR}")
message(STATUS "Install sysconfdir: ${CMAKE_INSTALL_FULL_SYSCONFDIR}")
message(STATUS "Install datadir:    ${CMAKE_INSTALL_FULL_DATADIR}")
