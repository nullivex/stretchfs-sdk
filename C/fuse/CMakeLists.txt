cmake_minimum_required(VERSION 3.10)
project(sfs_fuse C)

set(CMAKE_C_STANDARD 99)
# not using C++ / do this to avoid dumb warning about its non-use
set(CMAKE_CXX_COMPILER "")

###=> Setup paths
set(DEPS "${CMAKE_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH ${DEPS})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

###=> Find Dependencies
  #=>   ...Threads
find_package(Threads REQUIRED)
if(WIN32)
    set(USE_THREADS_WIN32 ON)
else()
    set(USE_THREADS_POSIX ${CMAKE_USE_PTHREADS_INIT})
    set(HAVE_PTHREAD_H ${CMAKE_USE_PTHREADS_INIT})
endif()
  #=>   ...CURL
find_package(CURL REQUIRED)
  #=>   ...FUSE
find_package(FUSE REQUIRED)

if(MSVC)
    # Microsoft compiler thinks standard C functions are insecure, and prints warnings
    #  but using their version of functions won't compile on standard gcc - NO THANKS
    message("Enabling -D_CRT_SECURE_NO_WARNINGS")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Enabling -DDEBUG, -Wall, and CMAKE_VERBOSE_MAKEFILE=ON")
    add_definitions(-DDEBUG)
    add_definitions(-Wall)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

###=> Define executable and includes/libraries
add_executable(sfs_fuse
        src/state.c include/state.h
        src/memory.c include/memory.h
        src/curl.c include/curl.h
        src/sfs_api.c include/sfs_api.h
        main.c
        )
target_include_directories(sfs_fuse PUBLIC
        "${DEPS}/include"
        "${FUSE_INCLUDE_DIRS}"
        )
target_link_libraries(sfs_fuse
        "${CMAKE_THREAD_LIBS_INIT}"
        "${DEPS}/lib/json-c.lib"
        "${CURL_LIBRARIES}"
        "${FUSE_LIBRARIES}"
        )