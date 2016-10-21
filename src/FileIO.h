/*
 * File:   FileIO.h
 * Author: kjell/weberr13
 *
 * https://github.com/weberr13/FileIO
 * Created on August 15, 2013, 2:28 PM
 */

#pragma once
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <sys/fsuid.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include "DirectoryReader.h"
#include "Result.h"

namespace FileIO {
void SetInterruptFlag(volatile int* interrupted = nullptr);
bool Interrupted();
Result<bool> IsMountPoint(const std::string& pathToDirectory);
Result<std::vector<uint8_t>> ReadBinaryFileContent(const std::string& pathToFile);
Result<std::string> ReadAsciiFileContent(const std::string& pathToFile);

Result<bool> WriteAppendBinaryFileContent(const std::string& filename, const std::vector<uint8_t>& content);
Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode);

Result<bool> ChangeFileOrDirOwnershipToUser(const std::string& path, const std::string& username);
bool DoesFileExist(const std::string& pathToFile);
bool DoesDirectoryExist(const std::string& pathToDirectory);
bool DoesDirectoryHaveContent(const std::string& pathToDirectory) ;

Result<bool> CleanDirectoryOfFileContents(const std::string& location, size_t& filesRemoved, std::vector<std::string>& foundDirectories);
Result<bool> CleanDirectory(const std::string& directory, const bool removeDirectory, size_t& filesRemoved);
Result<bool> CleanDirectory(const std::string& directory, const bool removeDirectory);
Result<bool> RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories);

Result<bool> RemoveFileAsRoot(const std::string& filename);
Result<std::string> ReadAsciiFileContentAsRoot(const std::string& filename);
bool MoveFile(const std::string& source, const std::string& dest);

Result<std::vector<std::string>> GetDirectoryContents(const std::string& directory);

struct passwd* GetUserFromPasswordFile(const std::string& username);
void SetUserFileSystemAccess(const std::string& username);

struct ScopedFileDescriptor {
   int fd;
   ScopedFileDescriptor(const std::string& location, const int flags, const int permission) {
      fd = open(location.c_str(), flags, permission);
   }

   ~ScopedFileDescriptor() {
      close(fd);
   }
};
}
