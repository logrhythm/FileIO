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

#include "Result.h"

namespace FileIO {
   void SetInterruptFlag(volatile int* interrupted = nullptr);    
   bool Interrupted();
   Result<bool> IsMountPoint(const std::string& pathToDirectory);
   Result<std::vector<char>> ReadBinaryFileContent(const std::string& pathToFile);
   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile);
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content);

   Result<bool> WriteAppendBinaryFileContent(const std::string& filename, const std::vector<char>& content); 
   Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode);
   Result<bool> ChangeFileOrDirOwnershipToUser(const std::string& path, const std::string& username);
   bool DoesFileExist(const std::string& pathToFile);
   bool DoesDirectoryExist(const std::string& pathToDirectory);
   
   Result<bool> CleanDirectoryOfFileContents(const std::string& location, size_t& filesRemoved, std::vector<std::string>& foundDirectories);
   Result<bool> CleanDirectory(const std::string& directory, const bool removeDirectory);
   Result<bool> RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories);
   
   Result<bool> RemoveFileAsRoot(const std::string& filename);
   Result<std::string> ReadAsciiFileContentAsRoot(const std::string& filename);
   bool MoveFile(const std::string& source, const std::string& dest);
   
   struct passwd* GetUserFromPasswordFile(const std::string& username);
   void SetUserFileSystemAccess(const std::string& username);

      /** TypeFound could be expanded. Ref /usr/include/dirent.h 
       * http://stackoverflow.com/questions/13132667/what-does-dt-wht-means-in-usr-include-dirent-h
       *  Anything that is not File or Directory will now be classified as Unknown
       * including link, device, unknown, pipe or fifo, socket and "linux whiteout" 
       * The values correspond to the enum values in /usr/include/bits/dirent.h except
       * for End which is set to one value higher than the maximum
      **/ 
   enum class FileType : unsigned char {Unknown=DT_UNKNOWN, Directory=DT_DIR, File=DT_REG, End=DT_WHT+1}; 
   struct DirectoryReader {
      typedef std::pair<FileType, std::string> Entry;
      explicit DirectoryReader(const std::string& pathToDirectory);
      ~DirectoryReader();

      Result<bool> Valid() {
         return mValid;
      }
      DirectoryReader::Entry Next();
      void Reset();

   private:
      DIR* mDirectory;
      struct dirent64 mEntry;
      struct dirent64* mResult;
      Result<bool> mValid;
   };   
   
   
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
