#.rst:
# FindFileIO
# -------
#
# Find libFileIO
# FileIO deals with file access and directory traversal.
##
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FileIO_INCLUDE_DIRS``
#   where to find DirectoryReader.h FileIO.h  FileSystemWalker.h  Result.h

#
# ``FileIO_LIBRARIES``
#   the libraries to link against to use libFileIO.
#   that includes libFileIO library files.
#
# ``FileIO_FOUND``
#   If false, do not try to use FileIO.
#
include(FindPackageHandleStandardArgs)
find_path(FileIO_INCLUDE_DIR DirectoryReader.h FileIO.h  FileSystemWalker.h  Result.h)
find_library(FileIO_LIBRARY
            NAMES libFileIO FileIO)

find_package_handle_standard_args(FileIO  DEFAULT_MSG
            FileIO_INCLUDE_DIR FileIO_LIBRARY)
            
mark_as_advanced(FileIO_INCLUDE_DIR FileIO_LIBRARY)
set(FileIO_LIBRARIES ${FileIO_LIBRARY})
set(FileIO_INCLUDE_DIRS ${FileIO_INCLUDE_DIR})