cmake_minimum_required(VERSION 3.10)
project(sfs_fuse C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(DEPS "${CMAKE_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH ${DEPS})

# Find Threads
find_package(Threads REQUIRED)
if(WIN32)
    set(USE_THREADS_WIN32 ON)
else()
    set(USE_THREADS_POSIX ${CMAKE_USE_PTHREADS_INIT})
    set(HAVE_PTHREAD_H ${CMAKE_USE_PTHREADS_INIT})
endif()
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

# Find CURL
find_package(CURL REQUIRED)
set(LIBS ${LIBS} ${CURL_LIBRARIES})


add_executable(sfs_fuse main.c)
target_include_directories(sfs_fuse PUBLIC
        "${DEPS}/include"
        )
target_link_libraries(sfs_fuse
        "${DEPS}/lib/libcurl.lib"
        "${DEPS}/lib/json-c.lib"
        )
