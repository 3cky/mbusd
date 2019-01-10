# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR armhf)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/arm-linux-gnueabihf-gcc)
## for c++ support `install g++-arm-linux-gnueabihf`
#SET(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# set the architecture for CPack (i.e packageing)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf)
