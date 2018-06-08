# Try to find fuse (devel)
# Once done, this will define
#
# FUSE_FOUND - system has fuse
# FUSE_INCLUDE_DIRS - the fuse include directories
# FUSE_LIBRARIES - fuse libraries directories

if(MSVC AND NOT FUSE_DIR)
    #Windows, no preloaded hint so assume default install location for WinFsp
    set(FUSE_DIR "C:/Program Files (x86)/WinFsp")
endif()

if(FUSE_INCLUDE_DIRS AND FUSE_LIBRARIES)
set(FUSE_FIND_QUIETLY TRUE)
endif(FUSE_INCLUDE_DIRS AND FUSE_LIBRARIES)

find_path( FUSE_INCLUDE_DIR fuse/fuse.h
        HINTS
        /usr
        ${FUSE_DIR}
        PATH_SUFFIXES include inc)

if(MacOSX)
    find_library( FUSE_LIBRARY osxfuse
            HINTS
            /usr/local/
            ${FUSE_DIR}
            PATH_SUFFIXES lib )
elseif(CMAKE_CL_64)
    find_library( FUSE_LIBRARY winfsp-x64
            HINTS
            ${FUSE_DIR}
            PATH_SUFFIXES lib )
elseif(CMAKE_CL)
    find_library( FUSE_LIBRARY winfsp-x86
            HINTS
            ${FUSE_DIR}
            PATH_SUFFIXES lib )
else()
    find_library( FUSE_LIBRARY fuse
            HINTS
            /usr
            ${FUSE_DIR}
            PATH_SUFFIXES lib )
endif()
set(FUSE_INCLUDE_DIRS ${FUSE_INCLUDE_DIR})
set(FUSE_LIBRARIES ${FUSE_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set FUSE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FUSE DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARY)

mark_as_advanced(FUSE_INCLUDE_DIR FUSE_LIBRARY)
