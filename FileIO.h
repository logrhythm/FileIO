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
<<<<<<< HEAD
#include <vector>
=======
>>>>>>> 79a9595d2d8ee34ab943349f72d55d0066e656d4
#include <fstream>
#include <dirent.h>
#include <sys/fsuid.h>
#include <unistd.h>
#include <pwd.h>

#include "include/global.h"

namespace FileIO {

   template<typename T> struct Result {
      const T result;
      const std::string error;

      /**
       * Result of a FileIO operation. 
       * @param output whatever the expected output would be
       * @param err error message to the client, default is empty which means successful operation
       */
      Result(T output, const std::string& err = {""})
      : result(output), error(err) {
      }

      /** @return status whether or not the Result contains a failure*/
      bool HasFailed() {
         return (!error.empty());
      }
   };

   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile);
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode);
   Result<bool> ChangeFileOrDirOwnershipToUser(const std::string& path, const std::string& username);
   bool DoesFileExist(const std::string& pathToFile);
   bool DoesDirectoryExist(const std::string& pathToDirectory);

   Result<bool> CleanDirectoryOfFileContents(const std::string& location, size_t& filesRemoved, std::vector<std::string>& foundDirectories);
   Result<bool> RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories);

   Result<bool> RemoveFileAsRoot(const std::string& filename);
   struct passwd* GetUserFromPasswordFile(const std::string& username);
   void SetUserFileSystemAccess(const std::string& username);

<<<<<<< HEAD
   /** TypeFound could be expanded. Ref /usr/include/dirent.h 
    * http://stackoverflow.com/questions/13132667/what-does-dt-wht-means-in-usr-include-dirent-h
    *  Anything that is not File or Directory will now be classified as Unknown
    * including link, device, unknown, pipe or fifo, socket and "linux whiteout" 
    * The values correspond to the enum values in /usr/include/bits/dirent.h except
    * for End which is set to one value higher than the maximum
    **/
   enum class FileType : unsigned char {
      Unknown = DT_UNKNOWN, Directory = DT_DIR, File = DT_REG, End = DT_WHT + 1
   };
=======
      /** TypeFound could be expanded. Ref /usr/include/dirent.h 
       * http://stackoverflow.com/questions/13132667/what-does-dt-wht-means-in-usr-include-dirent-h
       *  Anything that is not File or Directory will now be classified as Unknown
       * including link, device, unknown, pipe or fifo, socket and "linux whiteout" 
       * The values correspond to the enum values in /usr/include/bits/dirent.h except
       * for End which is set to one value higher than the maximum
      **/ 
   enum class FileType : unsigned char {Unknown=DT_UNKNOWN, Directory=DT_DIR, File=DT_REG, End=DT_WHT+1}; 
   
   struct DirectoryReader {

      //enum class TypeFound : unsigned char {Unknown=DT_UNKNOWN, Directory=DT_DIR, File=DT_REG, End=DT_WHT+1}; 
      typedef std::pair<FileType, std::string> Entry;
      explicit DirectoryReader(const std::string& pathToDirectory);
      ~DirectoryReader();         
      
      Result<bool> Valid() {return mValid;}
      DirectoryReader::Entry Next();
      void Reset();
      
   private:
      DIR* mDirectory;
      struct dirent64 mEntry;
      struct dirent64* mResult;
      Result<bool> mValid;
   };   
}
>>>>>>> 79a9595d2d8ee34ab943349f72d55d0066e656d4

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
}
