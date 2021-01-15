/*
 * File:   DirectoryReader.h
 * Author: KjellKod
 *
 * https://github.com/KjellKod/FileIO
 * Created on August 15, 2013, 2:28 PM
 */
#pragma once

#include <string>
#include <Result.h>
#include <dirent.h>
#include <pwd.h>

namespace FileIO {

/** TypeFound could be expanded. Ref /usr/include/dirent.h
* http://stackoverflow.com/questions/13132667/what-does-dt-wht-means-in-usr-include-dirent-h
*  Anything that is not File or Directory will now be classified as Unknown
* including link, device, unknown, pipe or fifo, socket and "linux whiteout"
* The values correspond to the enum values in /usr/include/bits/dirent.h except
* for End which is set to one value higher than the maximum
**/
enum class FileType : unsigned char {Unknown = DT_UNKNOWN, Directory = DT_DIR, File = DT_REG, End = DT_WHT + 1};

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
   struct dirent64* mResult;
   Result<bool> mValid;
};
}
