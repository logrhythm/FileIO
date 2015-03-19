/*
 * File:   DirectoryReader.cpp
 * Author: KjellKod
 *
 * https://github.com/KjellKod/FileIO
 * Created on August 15, 2013, 2:28 PM
 */

#include "DirectoryReader.h"
#include <cstring>
#include <cerrno>

/**
 * Helper lambda to instantiate the const Result<bool> AFTER
 * the opendir call.
 */
namespace {
/// @return the success of the opendir operation
auto DirectoryInit = [](DIR** directory, const std::string pathToDirectory) -> Result<bool> {
   *directory = opendir(pathToDirectory.c_str());
   std::string error{""};

   if (nullptr == *directory) {
      std::string error{std::strerror(errno)};
      return Result<bool> {false, error};
   }
   return Result<bool>(true);
};
} // anonymous helper



namespace FileIO {





DirectoryReader::DirectoryReader(const std::string& pathToDirectory)
   : mDirectory {nullptr }
   , mValid{DirectoryInit(&mDirectory, pathToDirectory)}{}

DirectoryReader::~DirectoryReader() {
   closedir(mDirectory);
}

/**
 * Finds the next entry in the directory or returns it.
 * The type of the entry is returned. The type corresponds to dirent.h d_type
 *
 * The only filesystem types supported are file and directory, all other will be classified as
 * TypeFound::Unknown
 *
 * The directories "." and ".." are skipped
 *
 * @return pair of name of entry and type. If end is reached "TypeFound::End" is returned.
 *
 */
DirectoryReader::Entry DirectoryReader::Next() {
   static const std::string Ignore1{"."};
   static const std::string Ignore2{".."};

   Entry entry = std::make_pair(FileType::Unknown, "");
   bool found = false;
   while (!found && (entry.first != FileType::End)) {
      readdir64_r(mDirectory, &mEntry, &mResult); // readdir_r is reentrant

      found = true; // abort immediately unless we hit "." or ".."

      if (nullptr == mResult) {
         entry = std::make_pair(FileType::End, "");
      } else if (static_cast<unsigned char> (FileType::Directory) == mEntry.d_type) {
         std::string name{mEntry.d_name};
         if ((Ignore1 == name) || (Ignore2 == name)) {
            found = false;
         } else {
            entry = std::make_pair(FileType::Directory, std::move(name));
         }
      } else if (static_cast<unsigned char> (FileType::File) == mEntry.d_type) {
         entry = std::make_pair(FileType::File, mEntry.d_name);
      } else {
         // Default case. Unless continue was called it will always exit here
         entry = std::make_pair(FileType::Unknown, "");
      }
   }
   return entry;
}
/** Resets the position of the directory stream to the beginning of the directory */
void DirectoryReader::Reset() {
   rewinddir(mDirectory);
}
}