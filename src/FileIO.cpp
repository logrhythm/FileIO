/* 
 * File:   FileIO.cpp
 * Author: kjell/weberr13
 * 
 * https://github.com/weberr13/FileIO
 * Created on August 15, 2013, 2:28 PM
 */



#include "FileIO.h"
#include "DirectoryReader.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <string>

namespace FileIO {
   volatile int* gFlagInterrupt = nullptr;
   
   
   /**
    * Makes it possible to set an interrupt handler flag to break possibly
    * lengthy operations such as MoveFile or CleanDirectoryOfFileContents
    * @param interrupted interrupt handler flag
    */
   void SetInterruptFlag(volatile int* interrupted) {
      gFlagInterrupt = interrupted;
   }
   
   /**
    * @return true if interrupt flag is initialized AND set to signal an interrupt
    */
   bool Interrupted() {
      if (nullptr == gFlagInterrupt) {
         return false;
      }
      
      return (*gFlagInterrupt > 0);
   }
   
   
   /**
    * Reads the stats for the path to see if it is a mountpoint
    * @param pathToDirectory that may be a mountpoint
    * @param returns Result<true> if it is a mountpoint
    */
   Result<bool> IsMountPoint(const std::string& pathToDirectory) {
      // A mount point must be a block-device, folder and with a separate device-ID from its ".." parent
      
      struct stat info;
      if (0 != stat(pathToDirectory.c_str(), &info)) {
         return Result<bool>{false, {"Cannot stat read location: " + pathToDirectory}};
      }
      
      if (!S_ISDIR(info.st_mode)) {
         return Result<bool>{false, {"Directory + [" + pathToDirectory + "] does not exist"}};
      }       
      
      std::string parentPath = pathToDirectory;
      if('/' != parentPath.back()){
         parentPath.append("/..");
      } else {
         parentPath.append("..");   
      }
     
      struct stat parent; 
      if (0 != stat(parentPath.c_str(), &parent)) {
         return Result<bool>{false, {"Cannot stat read parent location: [" + parentPath +"]"}};
      }
      
      //  st_dev: device number. st_ino: inode number
      const bool isMountPoint = (info.st_dev != parent.st_dev) || (info.st_dev == parent.st_dev && info.st_ino == parent.st_ino);
      if (!isMountPoint) {
         return Result<bool>{false, {"[" + parentPath + "] is not a mountpoint"}};
      }
      
      return Result<bool>{true};
   }

  /**
    * Reads content of binary  file
    * @param pathToFile to read
    * @return Result<std::vector<uint8_t>> all the content of the file, and/or an error string 
    *         if something went wrong 
    */
   Result<std::vector<uint8_t>> ReadBinaryFileContent(const std::string& pathToFile) {
      std::ifstream in(pathToFile, std::ifstream::binary);
      if (!in) {
         std::string error{"Cannot read-open file: "};
         error.append(pathToFile);
         return Result<std::vector<uint8_t>>{{}, error};
      }

      std::shared_ptr<void> fileCloser(nullptr, [&](void *) { // RAII file close
         if (in) {
            in.close();
         }
      }); //fileCloser RAII


      std::vector<char> contents;
      in.seekg(0, std::ios::end);
      auto end = in.tellg();
      in.seekg(0, std::ios::beg);

      // Attempt to read it the fastest way possible.
      if (-1 != end) {  // tellg() --> pos_type{-1} if reading the end failed.
         contents.resize(end);
         try {
            in.read(&contents[0], contents.size());
         } catch (std::iostream::failure& e) {
            std::string error{"Failed to read file: "};
            error += e.what();
            return Result<std::vector<uint8_t>>{{}, error};
         }
      } else {
         // Could not calculate with ifstream::tellg(). Is it a RAM file?
         // Fallback solution to slower iteratator approach
         contents.assign((std::istreambuf_iterator<char>(in)),
         (std::istreambuf_iterator<char>()));
      }

      // return copy of the read result.
      return Result<std::vector<uint8_t>>{{contents.begin(), contents.end()}};
   }


   /**
    * Reads content of Ascii file
    * @param pathToFile to read
    * @return Result<std::string> all the content of the file, and/or an error string 
    *         if something went wrong 
    */
   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile) {
      auto result = ReadBinaryFileContent(pathToFile);
      if(result.HasFailed()) {
         return Result<std::string>{{}, result.error};
      }

      std::string contents(reinterpret_cast<const char*> (result.result.data()), result.result.size());
      return Result<std::string>{contents};
   }



   /**
   * Write the serialized date to the given filename
   * @param filename
   * @param content
   * @return whether or not the write was successful
   */
   Result<bool> WriteAppendBinaryFileContent(const std::string & filename, const std::vector<uint8_t>& content) {
      
      static_assert(sizeof(char) == sizeof(uint8_t), "File writing assumes equal size for uint8_t and char");

      std::fstream outputFile;
      outputFile.open(filename.c_str(), std::fstream::out | std::fstream::binary | std::fstream::app);
      const bool openStatus = outputFile.is_open();
      std::shared_ptr<void> fileCloser(nullptr, [&](void *) { // RAII file close
         if (openStatus) {
            outputFile.close();
         }
      }); //fileCloser RAII

      std::string error = {"Unable to write test data to file: "};
      error.append(filename);

      if (!openStatus) {
         return Result<bool> {false, error};
      }

      //  FYI: thread_local must be trivial to initialize:
      //   http://coliru.stacked-crooked.com/view?id=6717cbf5974c0e5c
      const static int kOneMbBuffer = 1024 * 1024;
      thread_local char buffer[kOneMbBuffer];
      outputFile.rdbuf()->pubsetbuf(buffer, kOneMbBuffer);
      outputFile.exceptions(std::ios::failbit | std::ios::badbit); // trigger exception if error happens
      try {
         outputFile.write(reinterpret_cast<const char*>(&content[0]), content.size());
      } catch(const std::exception& e) {
        error.append(". Writing triggered exception: ").append(e.what());
        return Result<bool>{false, error};
      }

      return Result<bool>{true};
   }



   /** 
    * A generic write function that supports a variety of modes
    * @param pathToFile
    * @param content
    * @param mode, std::ios::app or std::ios::trunc
    * @return 
    */
   Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode) {
      std::ofstream out(pathToFile, std::ios::out | mode);
      if (!out) {
         std::string error{"Cannot write-open file: "};
         error.append(pathToFile);
         return Result<bool>{false, error};
      }

      std::shared_ptr<void> fileCloser(nullptr, [&](void *) { // RAII file close
         if (out) {
            out.close();
         }
      }); //fileCloser RAII

      out << content;
      return Result<bool>{true};
   }

   /**
    * Write ascii content to file
    * @param pathToFile to write
    * @param content to write to file
    * @return Result<bool> result if operation whent OK, if it did not the Result<bool>::error string 
    *         contains the error message
    */
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content) {

      return WriteFileContentInternal(pathToFile, content, std::ios::trunc);
   }

   /**
    * Write ascii content to the end of a file
    * @param pathToFile to write
    * @param content to write to file
    * @return Result<bool> result if operation whent OK, if it did not the Result<bool>::error string 
    *         contains the error message
    */
   Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content) {
      return WriteFileContentInternal(pathToFile, content, std::ios::app);
   }

   /**
    * Use stat to determine the presence of a file
    * @param pathToFile
    * @return if the stat command succeeded (meaning that there is something at that filename)
    */
   bool DoesFileExist(const std::string& pathToFile) {
      struct stat fileInfo;
      return (stat(pathToFile.c_str(), &fileInfo) == 0);
   }

   /**
    * Use stat to determine the presence of a directory
    * @param pathToFile
    * @return if the stat command succeeded (meaning that there is a directory that directory name)
    */
   bool DoesDirectoryExist(const std::string& pathToDirectory) {
      struct stat directoryInfo;
      if (0 != stat(pathToDirectory.c_str(), &directoryInfo)) {
         return false;
      }
      bool isDirectory = S_ISDIR(directoryInfo.st_mode);
      return isDirectory;
   }

   /**
    * Use to determine if the directory is empty or not
    * @param pathToDirectory
    * @return true if the directory is not empty, false otherwise
    */
   bool DoesDirectoryHaveContent(const std::string& pathToDirectory) {

      if (false == FileIO::DoesDirectoryExist(pathToDirectory)) {
         return false;
      }

      FileIO::DirectoryReader reader(pathToDirectory);
      auto entry = reader.Next();
      return (FileIO::FileType::End != entry.first);
   }

   
   
   /**
    * Iterate through the directory. Remove any file found,  save any found directory
    * Any attempt to delete files from "/"or "/root" will be ignored.
    * 
    * 
    * @param location to delete files from
    * @param return by reference number of files deleted
    * @param return by reference any found directory
    * @return whether or not all the operations were successful
    */
   Result<bool> CleanDirectoryOfFileContents(const std::string& location
           , size_t& filesRemoved, std::vector<std::string>& foundDirectories) {
      if (("/" == location) || ("/root" == location) || ("/root/" == location)) {
         return Result<bool>{false, {"Not allowed to remove directory: " + location}};
      }

      if (location.empty() || !FileIO::DoesDirectoryExist(location)) {
         return Result<bool>{false, {"Directory does not exist. False location was: " + location}};
      }

      FileIO::DirectoryReader reader(location);
      if (reader.Valid().HasFailed()) {
         return Result<bool>{false, {"Failed to read directory: " + location + ". Error: " + reader.Valid().error}};;
      }

      FileIO::DirectoryReader::Entry entry;
      size_t failures{0};
      filesRemoved = 0;
      std::string lastError;
      do {
         entry = reader.Next();
         if (FileIO::FileType::Directory == entry.first) {
            std::string dirPath{location};
            if ('/' != location.back()) {
               dirPath.append("/");
            }
            dirPath.append(entry.second);
            foundDirectories.push_back(dirPath);
         } else if (FileIO::FileType::File == entry.first) {
            const std::string pathToFile{location + "/" + entry.second};
            bool removedFile = (0 == unlink(pathToFile.c_str()));
            if (removedFile) {
               filesRemoved++;
            } else {
               ++failures;
               lastError = {"Last Error for file: " + pathToFile + ", errno: " };
               lastError.append(std::strerror(errno));
            }
         }
         // FileIO::FileSystem::Unknown is ignored
      } while (!Interrupted() && entry.first != FileIO::FileType::End);

      std::string report;
      if (failures > 0) {
         report = {"#" + std::to_string(failures) + " number of failed removals. " + lastError};
      }
      return Result<bool>{(0 == failures), report};
   }
   
   /**
   *  Iterate through the given directory location. Remove files and directories recursively until empty
   * @param directory, the directory to remove content from
   * @param removeDirectory, whether or not the start directory should be removed
   *  @return how many entities that were not removed. I.e. a successfull remove of all would have zero entities left
   */ 
   Result<bool> CleanDirectory(const std::string & directory, const bool removeDirectory, size_t& filesRemoved){
      std::string report;
      bool noFailures = true;

      std::vector<std::string> foundDirectories;
      auto cleanAllFiles = FileIO::CleanDirectoryOfFileContents(directory, filesRemoved, foundDirectories);

      if (cleanAllFiles.HasFailed()) {
         report = {"Failed to remove files from " +  directory};
         noFailures = false;
      } 
     

       while (foundDirectories.size() > 0){
          auto result = CleanDirectory(foundDirectories.back(), true);
          foundDirectories.pop_back();

          if (result.HasFailed()) {
             noFailures = false;
             report.append("\n").append(result.error);
          }
       }

       // no content should exist at this point. Safe to remove the directory
       if (removeDirectory) {
          auto result = FileIO::RemoveEmptyDirectories({{directory}});
          if (result.HasFailed()){
             noFailures = false;
             report.append("\n").append(result.error);
          }
       }
    
      return Result<bool> {noFailures, report};
   }
   
   Result<bool> CleanDirectory(const std::string & directory, const bool removeDirectory){
    size_t filesRemoved {0};
    return CleanDirectory(directory, removeDirectory, filesRemoved);
   }

   /**
    * Remove directories at the given paths
    * @return whether or not all the operations were successful
    */
   Result<bool> RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories) {
      std::ostringstream error;
      error << "Failed to remove given directories : ";
      bool success = true;
      for (const auto& directory : fullPathDirectories) {
         if (FileIO::DoesDirectoryExist(directory)) { // invalids are ignored. 
            int removed = rmdir(directory.c_str());
            if (0 != removed) {
               success = false;
               int errsv = errno;
               error << "\n" << directory << ", error: " << std::strerror(errsv);               
            }
         }
      }
      
      if (!success) {
         return Result<bool>{false, error.str()};
      }
      
      return Result<bool>{true};
   }
   
   
   /*
    * This function may need to be run with root file system privileges. In that case, it can be wrapped in
    * FileIO::SudoFile. For example:
    *    auto result = FileIO::SudoFile(FileIO::ChangeFileOrDirOwnershipToUser, location, "dpi");
    */
   Result<bool> ChangeFileOrDirOwnershipToUser(const std::string& path, const std::string& username) {

      struct passwd* pwd = GetUserFromPasswordFile(username);
      auto returnVal = chown(path.c_str(), pwd->pw_uid, pwd->pw_gid);
      int errsv = errno;
      free(pwd);
      if (returnVal < 0) {
         std::string error{"Cannot chown dir/file: " + path + " error number: " + std::to_string(errsv)};
         return Result<bool>{false, error};
      }
      return Result<bool>{true};
   }

   struct passwd* GetUserFromPasswordFile(const std::string& username) {
      // Get the uid for dpi user
      struct passwd* pwd = (struct passwd *) calloc(1, sizeof (struct passwd));
      if (pwd == NULL) {
         // Failed to allocate struct passwd for getpwnam_r.
         exit(1);
      }
      size_t buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof (char);
      char *buffer = (char *) malloc(buffer_len);
      if (buffer == NULL) {
         //Failed to allocate buffer for getpwnam_r.
         exit(2);
      }
      getpwnam_r(username.c_str(), pwd, buffer, buffer_len, &pwd);
      if (pwd == NULL) {
         //getpwnam_r failed to find requested entry.
         exit(3);
      }
      free(buffer);
      return pwd;
   }

   /*
    * When running as root, change the file system access to a user.
    */
   void SetUserFileSystemAccess(const std::string& username) {
      struct passwd* pwd = GetUserFromPasswordFile(username);

      setfsuid(pwd->pw_uid);
      setfsgid(pwd->pw_gid);

      free(pwd);

   }

   Result<bool> RemoveFile(const std::string& filename) {
      int rc = unlink(filename.c_str());

      if (rc == -1) {
         return Result<bool>{false, "Unable to unlink file"};
      }
      return Result<bool>{true};
   }
   
   /**
    * Move a file to another location. This works across device boundaries contrary to 
    * 'rename'
    * 
    * @return true if moved successfully and deleted the previous file successfully
    *         false if move fail or the original file could not be deleted. For all failure 
    *         cases errno will be set
    *
    * Reference: 
    * http://linux.die.net/man/3/rename
    * http://linux.die.net/man/3/open
    * http://www.lehman.cuny.edu/cgi-bin/man-cgi?sendfile+3
    * 
    * Performance sendfile vs user-level copy: 
    * http://stackoverflow.com/questions/10195343/copy-a-file-in-an-sane-safe-and-efficient-way
    */
   bool MoveFile(const std::string& sourcePath, const std::string& destPath) {      
      if (!DoesFileExist(sourcePath) || sourcePath == destPath) {
         return false; // DoesFileExist sets errno: ENOENT i.e. No such file or directory
      }
     
      int64_t rc = rename(sourcePath.c_str(), destPath.c_str());
      
      if (-1 == rc) {
         // On a separate device. Clear errno and try with sendfile
         errno = 0;
         ScopedFileDescriptor src(sourcePath, O_RDONLY, 0);
         struct stat stat_src;
         fstat(src.fd, &stat_src);
         const unsigned int permissions = stat_src.st_mode; 

         ScopedFileDescriptor dest(destPath, O_WRONLY | O_CREAT, permissions);
         off_t offset = 0; // byte offset for sendfile
         
         // sendfile returns -1 for failure else number of bytes sent
         // sendfile can only move std::numeric_limits<int>::max() each time
         //           i.e  2147483647 bytes (1.999... GB)
         // 'sendfile' is repeatedly called until all of the file is copied
         rc = sendfile(dest.fd, src.fd, &offset, stat_src.st_size);
         while (rc > 0 && rc < stat_src.st_size && !Interrupted())  {
            rc += sendfile(dest.fd, src.fd, &offset, stat_src.st_size);
         }  
         
         // for certain file permissions unit testing gave that the permissions
         // were not preserved over the sendfile call. If this is the case
         // we re-set the same permissions as the original file had
         struct stat stat_dest;
         fstat(dest.fd, &stat_dest);
         int64_t rcStat = {0};
         if (permissions != (stat_dest.st_mode & permissions)) {
            rcStat = chmod(destPath.c_str(), permissions);
         }
         
         
         if (-1 != rc && rc == stat_src.st_size && (0 == rcStat)) {
            rc = remove(sourcePath.c_str());
         }
      }

      return (0 == rc);
   }

   Result<std::vector<std::string>> GetDirectoryContents(const std::string& directory) {
     std::vector<std::string> filesInDirectory;
     DIR* dir;
     struct dirent* entry;
     if ((dir = opendir (directory.c_str())) != NULL) {
       while ((entry = readdir (dir)) != NULL) {
         if (entry->d_type == DT_REG) {
            filesInDirectory.push_back(std::string(entry->d_name));
         }
       }
       closedir (dir);
     } else {
       return Result<std::vector<std::string>>{{}, "ERROR:  Could not open directory for reading"};
     }
     return Result<std::vector<std::string>>{filesInDirectory, ""};
   }

} // namespace FileIO


