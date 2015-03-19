/* 
 * File:   FileSystemWalker.cpp
 * Author: kjell
 *
 * https://github.com/weberr13/FileIO
 * Created on April 21, 2014
 */

#include "FileSystemWalker.h"
#include "FileIO.h"
#include <sys/types.h>
#include <cerrno>
#include <cstring> 


namespace {
   struct ScopedFts {
      FTS* mFts;

      explicit ScopedFts(FTS* fts) : mFts(fts) { }

      ~ScopedFts() {
         fts_close(mFts);
      }

      ScopedFts() = delete;
      ScopedFts(const FileSystemWalker&) = delete;
      ScopedFts& operator=(const FileSystemWalker&) = delete;

   };
}

/**
 * @param startPath to traverse the directory at
 * @param ftsHandler a function that takes FTSENT* and flag of filetype for file system processing
 */
FileSystemWalker::FileSystemWalker(const std::string& startPath, std::function<int(FTSENT*, int) > ftsHandler) \
: mFtsHandler(ftsHandler)
, mStartPath(startPath) {
 }


/**
 * @return true if the directory exist
 */
bool FileSystemWalker::IsValid() const {
   return FileIO::DoesDirectoryExist(mStartPath);
}


/**
 * Walks through the whole directory tree, starting at the given path.  It will 
 * call the @ref mFtsHandler for EVERY file or directory that it is encountered. 
 * 
 * It will stop when the FTSENT handling function returns a non-zero value or when 
 * the directory tree is exhausted and there are no more entries to find
 * @result Result<int> with traversal status and any possible error messages
 *
 * Some Useful FTSENT Flags
 * FTS_D      A directory being visited in preorder.
 *
 * FTS_DC     A directory that causes a cycle in the tree.
 *            (The fts_cycle field of the FTSENT structure
 *            will be filled in as well.)
 *
 * FTS_DNR    A directory which cannot be read.  This is an
 *            error return, and the fts_errno field will be
 *            set to indicate what caused the error.
 *
 * FTS_DOT    A file named "."  or ".."  which was not
 *            specified as a filename to fts_open() (see
 *            FTS_SEEDOT).
 *
 * FTS_DP     A directory being visited in postorder.  The
 *            contents of the FTSENT structure will be
 *            unchanged from when it was returned in
 *            preorder, that is, with the fts_info field
 *            set to FTS_D.
 *
 * FTS_F      A regular file.
 *
 * Example usage:
   @verbatim
 // Example of how to count all file system entities from a given path
 // The saved results is saved INSIDE the given FTSENT handler
  void SomeFunc() {
    size_t entityCounter;
    std::function<int(FTSENT*,int> CountFiles= [&](FTSENT *ptr, int flag) { ++entityCounter;}; 
    FileSystemWalker walker(path, CountFiles);
    walker.Action(); // this will increment entityCounter until the file path is exhausted
  @endvarbatim
 */
Result<int> FileSystemWalker::Action() {
   if (!IsValid()) {
      return Result<int>{-1, {"Invalid Path: " + mStartPath}};
   }
   char* rawpath = const_cast<char*> (mStartPath.c_str());
   char* ftsRawPath[] = {rawpath, nullptr}; // convert path to "char* const* "

   // fts_open arguments:
   //      FTS_PHYSICAL: Symbolic links will NOT be followed
   //      FTS_XDEV : will not go to a different mount point then the starting point
   //      FTS_NOCHDIR: IMPORTANT:   This flag is needed for thread safety. 
   //                    Internally it will NOT do any chdir calls with this flag, 
   //                    but it will slow down the execution a bit
   //      nullptr     : no specific ordering is made for returning the results
   // Reference: http://man7.org/linux/man-pages/man3/fts.3.html
   FTS* file_system = fts_open(ftsRawPath, FTS_PHYSICAL | FTS_XDEV | FTS_NOCHDIR, nullptr);
   if (nullptr == file_system) {
      return Result<int>{-1, {"Could not open filesystem at: " + mStartPath}};
   }

   ScopedFts cleanup(file_system);
   FTSENT* node = nullptr;
   int status = 0;
   while ((0 == status) && (node = fts_read(file_system)) != nullptr) {
      int info = node->fts_info;
      status = mFtsHandler(node, info);
   }

   static auto GetError = [](const int status, const std::string& path) -> std::string {
      if (-1 == status) {
         std::string error = {"Error occurred during file access starting from: " + path + ", error: " + std::strerror(errno)};
         return error;
      }
      return {};
   };
   
   return Result<int>{status, GetError(status, mStartPath)};
}
