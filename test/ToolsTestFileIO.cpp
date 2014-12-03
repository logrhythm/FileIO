/* 
 * File:   ToolsTestFileIO.cpp
 * Author: kjell
 *
 * Created on August 15, 2013, 3:41 PM
 */

#include <random>
#include <cstdio>
#include <functional>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <g2log.hpp>
#include <future>
#include <thread>
#include <random>
#include <sstream>
#include <unistd.h>
#include "ToolsTestFileIO.h"
#include "FileIO.h"
#include "StopWatch.h"

namespace {
   // Random integer function from http://www2.research.att.com/~bs/C++0xFAQ.html#std-random

   int random_int(int low, int high) {
      using namespace std;
      static std::random_device rd; // Seed with a real random value, if available
      static default_random_engine engine{rd()};
      typedef uniform_int_distribution<int> Distribution;
      static Distribution distribution{};
      return distribution(engine, Distribution::param_type{low, high});
   }
   struct ScopedFileCleanup {
      const std::string file;

      explicit ScopedFileCleanup(const std::string& name) : file(name) { }

      ~ScopedFileCleanup() {
         remove(file.c_str());
      }
   };
   

   std::string GetCurrentDirectory() {
      char* rawdir = get_current_dir_name();
      std::string dir{rawdir};
      free(rawdir);
      return dir;
   }
} // anonymous


void  FileMover(const std::string& from, const std::string& to, const int permissions, const size_t kbytes) {      
      ScopedFileCleanup scopedFrom(from);
      ScopedFileCleanup scopedTo(to);
     
      EXPECT_TRUE(FileIO::DoesFileExist(from));
      struct stat fromStat;
      stat(from.c_str(), &fromStat);
      EXPECT_EQ(fromStat.st_size, (1024 * kbytes));
      EXPECT_EQ(fromStat.st_mode & permissions, permissions);
      
      // Move file
      std::this_thread::sleep_for(std::chrono::milliseconds(random_int(0, 100)));
      auto moved = FileIO::MoveFile(from, to);
      EXPECT_TRUE(moved);
      EXPECT_FALSE(FileIO::DoesFileExist(from));
      EXPECT_TRUE(FileIO::DoesFileExist(to));

      // verify size and permissions     
      struct stat toStat;
      stat(to.c_str(), &toStat);
      EXPECT_EQ(toStat.st_size, (1024 * kbytes));
      EXPECT_EQ(toStat.st_mode & permissions, permissions) << "file: " << from << " : " << to;
   }


// NO OTHER TEST FUNCTION SHOULD PRECEDE THIS FUNCTION!
TEST_F(TestFileIO, Interrupt_null) {
   EXPECT_FALSE(FileIO::Interrupted());
   volatile int* nullflag = nullptr;
   FileIO::SetInterruptFlag(nullflag);
   EXPECT_FALSE(FileIO::Interrupted());
}
TEST_F(TestFileIO, Interrupt_happens) {
   EXPECT_FALSE(FileIO::Interrupted());
   volatile int flag = 0;
   FileIO::SetInterruptFlag(&flag);
   EXPECT_FALSE(FileIO::Interrupted());
   flag = 1;
   EXPECT_TRUE(FileIO::Interrupted());
   
   
   FileIO::SetInterruptFlag(/*nullptr*/);
   EXPECT_FALSE(FileIO::Interrupted());
}




TEST_F(TestFileIO, TestOfTestUtility) {
   std::string file1 = CreateFile(mTestDirectory, "ToBeRemoved.txt");
   EXPECT_TRUE(FileIO::DoesFileExist(file1));
   { 
      ScopedFileCleanup cleaner(file1);
   }
   EXPECT_FALSE(FileIO::DoesFileExist(file1));  
}

TEST_F(TestFileIO, CannotOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/xyz/*&%/x.y.z"});
   EXPECT_TRUE(fileRead.result.empty());
   EXPECT_FALSE(fileRead.error.empty());
   EXPECT_TRUE(fileRead.HasFailed());
}

TEST_F(TestFileIO, CanOpenFileToRead) {
   auto fileRead = FileIO::ReadAsciiFileContent({"/proc/stat"}); // fine as long as we are on *nix systems
   EXPECT_FALSE(fileRead.result.empty());
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}

TEST_F(TestFileIO, CannotWriteToFile) {
   auto fileWrite = FileIO::WriteAsciiFileContent({"xyz/123/proc/stat"},
   {
      "Hello World"
   });
   EXPECT_FALSE(fileWrite.result);
   EXPECT_FALSE(fileWrite.error.empty());
   EXPECT_TRUE(fileWrite.HasFailed());
}

TEST_F(TestFileIO, CanWriteToFileAndReadTheFile) {
   using namespace std;

   string filename{"/tmp/TestFileIO_"};
   filename.append(to_string(random_int(0, 1000000)))
           .append({"_"})
   .append(to_string(random_int(0, 1000000)));

   // cleanup/removing the created file when exiting
   ScopedFileCleanup cleanup{filename};

   auto fileWrite = FileIO::WriteAsciiFileContent(filename,{"Hello World"});
   EXPECT_TRUE(fileWrite.result);
   EXPECT_TRUE(fileWrite.error.empty());
   EXPECT_FALSE(fileWrite.HasFailed());

   auto fileRead = FileIO::ReadAsciiFileContent(filename);
   EXPECT_EQ("Hello World", fileRead.result);
   EXPECT_TRUE(fileRead.error.empty());
   EXPECT_FALSE(fileRead.HasFailed());
}

TEST_F(TestFileIO, FileIsNotADirectory) {
   std::string filename{"/tmp/123_456_789"};
   {
      ScopedFileCleanup cleanup{filename};
      EXPECT_FALSE(FileIO::DoesFileExist(filename));
      EXPECT_FALSE(FileIO::DoesDirectoryExist(filename));

      auto fileWrite = FileIO::WriteAsciiFileContent(filename,{"Hello World"});
      EXPECT_TRUE(fileWrite.result);
      EXPECT_TRUE(fileWrite.error.empty());
      EXPECT_FALSE(fileWrite.HasFailed());

      EXPECT_TRUE(FileIO::DoesFileExist(filename));
      EXPECT_FALSE(FileIO::DoesDirectoryExist(filename));
   }
   // RAII cleanup
   EXPECT_FALSE(FileIO::DoesFileExist(filename));

}

TEST_F(TestFileIO, DirectoryExistance) {
   std::string directory{"/tmp/some_temp_directory"};
   {
      EXPECT_FALSE(FileIO::DoesDirectoryExist(directory)) << directory;
      std::string createDir{"mkdir -p " + directory};
      EXPECT_EQ(0, system(createDir.c_str()));

      ScopedFileCleanup cleanup{directory};
      EXPECT_TRUE(FileIO::DoesFileExist(directory));
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
   }
   // RAII cleanup
   EXPECT_FALSE(FileIO::DoesFileExist(directory));
   EXPECT_FALSE(FileIO::DoesDirectoryExist(directory));
}

TEST_F(TestFileIO, DirectoryIsEmpty) {
   std::string directory{"/tmp/some_temp_directory"};
   {
      EXPECT_FALSE(FileIO::DoesDirectoryExist(directory)) << directory;
      std::string createDir{"mkdir -p " + directory};
      EXPECT_EQ(0, system(createDir.c_str()));

      ScopedFileCleanup cleanup{directory};
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
      EXPECT_FALSE(FileIO::DoesDirectoryHaveContent(directory));
   }
}

TEST_F(TestFileIO, DirectoryIsNotEmpty) {
   std::string directory{"/tmp/some_temp_directory"};
   {
      EXPECT_FALSE(FileIO::DoesDirectoryExist(directory)) << directory;
      std::string createDir{"mkdir -p " + directory};
      EXPECT_EQ(0, system(createDir.c_str()));

      ScopedFileCleanup cleanup{directory};
      EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
      std::string file = directory + "/fakefile.txt";
      FileIO::WriteAsciiFileContent(file, "");
      EXPECT_TRUE(FileIO::DoesDirectoryHaveContent(directory));
      FileIO::RemoveFileAsRoot(file);
   }
}

TEST_F(TestFileIO,  RemoveEmptyDirectories_FakeDirectoriesAreIgnored) {
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({}).result); // invalids are ignored
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({{""}}).result); // invalids are ignored

   std::vector<std::string> doNotExist({{}, {" "}, {"/does/not/exist"}});
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories(doNotExist).result); // invalids are ignored
   
   CreateSubDirectory("some_directory");
   std::string real = {mTestDirectory+"/"+"some_directory"};
   EXPECT_TRUE(FileIO::DoesDirectoryExist(real));
   EXPECT_TRUE(FileIO::RemoveEmptyDirectories({{"does_not_exist"},real}).result); // the one invalid is ignored
   
}

TEST_F(TestFileIO, RemoveDirectories__ExpectNonEmptyToStay) {
   CreateSubDirectory("some_directory");
   CreateFile({mTestDirectory + "/some_directory/"}, "some_file");
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/some_directory/some_file"}));
   EXPECT_FALSE(FileIO::RemoveEmptyDirectories({{mTestDirectory + "/some_directory"}}).result);
   EXPECT_TRUE(FileIO::DoesFileExist({mTestDirectory + "/some_directory/some_file"}));   
}

TEST_F(TestFileIO, MoveFiles__BogusFileCannotBeMoved) {
   auto dir1 = CreateSubDirectory("some_directory1");
   auto dir2 = CreateSubDirectory("some_directory2");
   
   auto moved = FileIO::MoveFile({dir1 + "/bogus"}, {dir2 + "/bogus"});
   std::string error {std::strerror(errno)};

   EXPECT_FALSE(moved);
   EXPECT_FALSE(FileIO::DoesFileExist({dir1 + "/bogus"}));
   EXPECT_FALSE(FileIO::DoesFileExist({dir2 + "/bogus"}));
   EXPECT_TRUE(std::string::npos != error.find("No such file or directory")) << error;
}




TEST_F(TestFileIO, MoveFiles__FileCanBeMovedAcrossDirectories) {
   auto dir1 = CreateSubDirectory("some_directory1");
   auto dir2 = CreateSubDirectory("some_directory2");

   std::string file1_5MBPath = {dir1 + "/file1_5MB"};    
   std::string createFile5MB = "dd bs=1024 count=5120 if=/dev/zero of=";
   std::string createFile1 = {createFile5MB + file1_5MBPath};
   EXPECT_EQ(0, system(createFile1.c_str()));
   const int file1Permissions = 644;
   EXPECT_TRUE(0 == chmod(file1_5MBPath.c_str(), file1Permissions));
   
   // Verify creation
   EXPECT_TRUE(FileIO::DoesFileExist(file1_5MBPath));
   struct stat file1Stat;
   stat(file1_5MBPath.c_str(), &file1Stat);
   EXPECT_EQ(file1Stat.st_size, (1024*5120));
   EXPECT_EQ(file1Stat.st_mode & file1Permissions, file1Permissions);
   EXPECT_NE(file1Stat.st_mode & 555, 555); // sanity check   
   
   
   // Move   
   std::string movedFile1_5MBFile = {dir2 + "/moved5MBFile"};
   auto moved = FileIO::MoveFile(file1_5MBPath, movedFile1_5MBFile);
   EXPECT_TRUE(moved);
   EXPECT_FALSE(FileIO::DoesFileExist(file1_5MBPath));
   EXPECT_TRUE(FileIO::DoesFileExist(movedFile1_5MBFile));

   // verify size and permissions
   struct stat fileMovedStat;
   stat(movedFile1_5MBFile.c_str(), &fileMovedStat);
   EXPECT_EQ(fileMovedStat.st_size, (1024*5120));
   EXPECT_EQ(fileMovedStat.st_mode & file1Permissions, file1Permissions);
   EXPECT_NE(fileMovedStat.st_mode & 555, 555); // sanity check   
}





TEST_F(TestFileIO, SYSTEM__MoveFiles__ThreadSafeMoveOfFiles) {
   std::string oldStorage = "/tmp";
   std::string newStorage = GetCurrentDirectory();
   if (false == FileIO::DoesDirectoryExist(oldStorage)) {
      SUCCEED() << "Skipping test. Cannot run test. Directory does not exist: " << oldStorage;
      return;
   }

   if (false == FileIO::DoesDirectoryExist(newStorage)) {
      SUCCEED() << "Skipping test. Cannot run test. Directory does not exist: " << newStorage;
      return;
   }

   // Verify that they are on different devices
   struct stat stat_path1;
   struct stat stat_path2;
   stat(oldStorage.c_str(), &stat_path1);
   stat(newStorage.c_str(), &stat_path2);
   if (stat_path1.st_dev == stat_path2.st_dev) {
      std::string warning = "SKIPPING TEST. CANNOT RUN TEST FOR VERIFYING MOVE OF FILES ACROSS DEVICES";
      std::cout << warning << std::endl;
      LOG(WARNING) << warning;
      SUCCEED() << warning;
      return;
   } 


   // Create all the files. Put a given size to them and vary the permissions
   const size_t movesToMake = 250;
   for (size_t i = 0; i < movesToMake; ++i) {
      const size_t kbytes = i*10;
      std::string from{oldStorage + "/from_" + std::to_string(i)};
      std::string createFile = {"dd bs=1024 count=" + std::to_string(kbytes) + " if=/dev/zero of=" + from + " >& /dev/null"}; // skip printf from dd
      EXPECT_EQ(0, system(createFile.c_str()));
      int permissions = 777;
      if (0 == i%2) {
         permissions = 640;
      } else if (0 == i%3) {
         permissions = 555;
      }
         
      // Verify creation
      EXPECT_TRUE(FileIO::DoesFileExist(from)); 
      EXPECT_TRUE(0 == chmod(from.c_str(), permissions));

      struct stat filestat;
      stat(from.c_str(), &filestat);
      EXPECT_EQ(filestat.st_size, (1024 * kbytes));
      EXPECT_EQ(filestat.st_mode & permissions, permissions);
   }
      
      
   std::vector<std::future<void>> result;
   for (size_t i = 0; i < movesToMake; ++i) {
      std::string from{oldStorage + "/from_" + std::to_string(i)};
      std::string to{newStorage + "/to_" + std::to_string(i)};
      int permissions = 777;
      if (0 == i%2) {
         permissions = 640;
       } else if (0 == i%3) {
         permissions = 555;
      }
         
      const size_t kbytes = i*10;
      result.push_back(std::async(std::launch::async, FileMover, from, to, permissions, kbytes));
   }

   for (auto& res : result) {
      res.wait();
   }
}
   
   

//
// Moving a large file is SLOW,. however this is faster than for example the system
// 'mv' command.  On my box the actual FileIO::MoveFile takes about 2.6 seconds
// while the system mv command takes 3.40 seconds
//
// Example
// sudo dd bs=1024 count=3145728 if=/dev/sdb of=/tmp/hej.txt
// time sudo mv /tmp/hej.txt  ~/.
//  
//   real    0m3.430s
//   user    0m0.021s
//   sys     0m3.401s
TEST_F(TestFileIO, SYSTEM__MoveFiles__LargeFileCanBeMovedAcrossDevices) {
   std::string oldStorage = "/tmp";
   std::string newStorage = GetCurrentDirectory();
   if (false == FileIO::DoesDirectoryExist(oldStorage)) {
       SUCCEED() << "Skipping test. Cannot run test. directory does not exist: " << oldStorage;
       return;
   }

   if (false == FileIO::DoesDirectoryExist(newStorage)) {
      SUCCEED() << "Skipping test. Cannot run test. directory does not exist: " << newStorage;
      return;
   }


   // Verify that they are on different devices
   struct stat stat_path1;
   struct stat stat_path2;
   stat(oldStorage.c_str(), &stat_path1);
   stat(newStorage.c_str(), &stat_path2);
   if (stat_path1.st_dev == stat_path2.st_dev) {
      std::string warning = "SKIPPING TEST. CANNOT RUN TEST FOR VERIFYING MOVE OF FILES ACROSS DEVICES";
      std::cout <<  warning << std::endl;
      LOG(WARNING) << warning;
      SUCCEED() << warning;
   } else {
      
      //
      // We have two devices to test moving a large file between. 
      // 3GB will be moved
  
      
      // Create the 3GB file
      std::string file1_3GBPath = {oldStorage + "/file1_3GB"};
      // use dd to write   (possibly)non zeroed data to the file
      std::string createFile3GB = {"dd bs=1024 count=3145728 if=/dev/zero of="}; // 3GB : 3145728 * 1024 bytes 
      std::string createFile1 = {createFile3GB + file1_3GBPath};
      ScopedFileCleanup fileOldCleaner(file1_3GBPath);
      EXPECT_EQ(0, system(createFile1.c_str()));
      const int file1Permissions = 644;
      EXPECT_TRUE(0 == chmod(file1_3GBPath.c_str(), file1Permissions));
      EXPECT_TRUE(FileIO::DoesFileExist(file1_3GBPath));
      
      
      // Verify that the file was created as big as we expected
      // Set test unique file permissions on the file
      struct stat64 file1Stat;
      stat64(file1_3GBPath.c_str(), &file1Stat);
      EXPECT_EQ(file1Stat.st_size, int64_t{3L * 1024L * 1024L * 1024L});    
      EXPECT_EQ(file1Stat.st_mode & file1Permissions, file1Permissions);
      EXPECT_NE(file1Stat.st_mode & 555, 555); // sanity check  
      std::string checksumFile1Result = {mTestDirectory + "/1_chksum"};
      std::string checksumOnFile1 {"cksum  " + file1_3GBPath + " > " + checksumFile1Result};
      EXPECT_EQ(0, system(checksumOnFile1.c_str()));
      std::string checksum1AsString = FileIO::ReadAsciiFileContent(file1_3GBPath).result;
              

      
      // Move it to the other device
      std::string movedFile1_3GBFile = {newStorage + "/moved3GBFile"};
      ScopedFileCleanup fileNewCleaner(movedFile1_3GBFile);
      
      std::cout << "\tStart moving file: " << file1_3GBPath << "  to " << movedFile1_3GBFile << std::endl;
      StopWatch watch;
      auto moved = FileIO::MoveFile(file1_3GBPath, movedFile1_3GBFile);
      std::cout << "\tMoving file: " << file1_3GBPath << "  to " << movedFile1_3GBFile << ", took: " << float(watch.ElapsedMs())/1000 << " sec" << std::endl;
      EXPECT_TRUE(moved) << "std::strerror(errno): " << std::strerror(errno) 
              << "\n file1: " << file1_3GBPath << ":" << FileIO::DoesFileExist(file1_3GBPath) 
              << "\n file2: " << movedFile1_3GBFile << ":" << FileIO::DoesFileExist(movedFile1_3GBFile);
              
      EXPECT_FALSE(FileIO::DoesFileExist(file1_3GBPath));
      EXPECT_TRUE(FileIO::DoesFileExist(movedFile1_3GBFile));
      

      // verify size and permissions and integrity of the file
      struct stat64 fileMovedStat;
      stat64(movedFile1_3GBFile.c_str(), &fileMovedStat);
      EXPECT_EQ(fileMovedStat.st_size, size_t{3L * 1024L * 1024L * 1024L});
      EXPECT_EQ(fileMovedStat.st_mode & file1Permissions, file1Permissions);
      EXPECT_NE(fileMovedStat.st_mode & 555, 555); // sanity check   
      
      // Verify checksums
      std::string checksumFile2Result = {mTestDirectory + "/2_chksum"};
      std::string checksumOnFile2 {"cksum " + movedFile1_3GBFile + " > " + checksumFile2Result};
      EXPECT_EQ(0, system(checksumOnFile2.c_str()));
      std::string checksum2AsString = FileIO::ReadAsciiFileContent(movedFile1_3GBFile).result;
      EXPECT_EQ(checksum1AsString,checksum2AsString);
   }
}








TEST_F(TestFileIO, CleanDirectoryOfFileContents_BogusDirectory) {
   std::vector<std::string> newDirectories;
   size_t removedFiles{0};   
   EXPECT_FALSE(FileIO::CleanDirectoryOfFileContents("", removedFiles, newDirectories).result);
   EXPECT_FALSE(FileIO::CleanDirectoryOfFileContents("/does/not/exist/", removedFiles, newDirectories).result);
}

TEST_F(TestFileIO, CleanDirectoryOfFileContents) {   
   std::vector<std::string> newDirectories;
   size_t removedFiles{0};  
   CreateSubDirectory("some_directory");
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"}));
   EXPECT_EQ(removedFiles, 0);
   
   CreateFile(mTestDirectory, "some_file");  
   EXPECT_TRUE(FileIO::CleanDirectoryOfFileContents(mTestDirectory, removedFiles, newDirectories).result);
   EXPECT_EQ(removedFiles, 1);
   ASSERT_EQ(newDirectories.size(), 1);
   EXPECT_EQ(std::string{mTestDirectory + "/some_directory"}, newDirectories[0]);
   // directories are not removed
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"})); 
}

TEST_F(TestFileIO, CleanDirectoryOfFileContents__StopsWhenInterrupted) {   
   std::vector<std::string> newDirectories;
   size_t removedFiles{0};  

   EXPECT_EQ(removedFiles, 0);   
   volatile int interrupted = 1;
   FileIO::SetInterruptFlag(&interrupted);
   
   CreateFile(mTestDirectory, "some_file1");  
   CreateFile(mTestDirectory, "some_file2");  
   EXPECT_TRUE(FileIO::CleanDirectoryOfFileContents(mTestDirectory, removedFiles, newDirectories).result);
   FileIO::SetInterruptFlag(); // reset before we can exit the test
   EXPECT_EQ(removedFiles, 1); // NOT TWO files since we interrupted it
}

TEST_F(TestFileIO, CleanDirectory__NoDirectory__ExpectFailure) {
   std::string nonsense = "/bla/bla/bla/does/not/exist";
   const bool removeStartDirectory = true;
   const bool keepStartDirectory = false;
   auto result = FileIO::CleanDirectory(nonsense, removeStartDirectory);
   EXPECT_TRUE(result.HasFailed());
   auto result2 = FileIO::CleanDirectory(nonsense, keepStartDirectory);
   EXPECT_TRUE(result2.HasFailed());

   // same for a file
   auto file = CreateFile(mTestDirectory, "file.txt");
   EXPECT_TRUE(FileIO::DoesFileExist(file));
   auto result3 = FileIO::CleanDirectory(file, keepStartDirectory);
   EXPECT_TRUE(result3.HasFailed());
}

TEST_F(TestFileIO, CleanDirectoryOfFilesAndDirectories) {
   const std::string baseDir = CreateSubDirectory("base");
   ASSERT_TRUE(FileIO::DoesDirectoryExist(baseDir));


   auto createContent = [&]() {
      // Create a bunch of directories. both with content and emtpy
      std::string currentLevel = baseDir;
      for (size_t index = 0; index < 100; ++index){
         std::string dir1 = CreateSubDirectory("dir1", currentLevel);
         std::string dir2 = CreateSubDirectory("dir2", currentLevel);
         EXPECT_TRUE(FileIO::DoesDirectoryExist(dir1));
         EXPECT_TRUE(FileIO::DoesDirectoryExist(dir2));

         std::string file = CreateFile(dir1, "file.txt");
         EXPECT_TRUE(FileIO::DoesFileExist(file));
         currentLevel = dir1;
      }
   };

   
   createContent();
   EXPECT_TRUE(FileIO::DoesDirectoryExist({baseDir + "/dir1"}));
   EXPECT_TRUE(FileIO::DoesDirectoryExist({baseDir + "/dir2"}));
   EXPECT_TRUE(FileIO::DoesFileExist({baseDir + "/dir1/file.txt"}));


   // keep base dir
   const bool keepStartDirectory = false;
   auto result1 = FileIO::CleanDirectory(baseDir, keepStartDirectory);
   EXPECT_EQ(result1.HasFailed(), false);
   EXPECT_FALSE(FileIO::DoesDirectoryExist({baseDir + "/dir1"}));
   EXPECT_FALSE(FileIO::DoesDirectoryExist({baseDir + "/dir2"}));
   EXPECT_FALSE(FileIO::DoesFileExist({baseDir + "/dir1/file.txt"}));
   EXPECT_TRUE(FileIO::DoesDirectoryExist(baseDir)); // BASE DIR IS KEPT


   // remove base dir
   createContent();
   const bool removeStartDirectory = true;
   auto result2 = FileIO::CleanDirectory(baseDir, removeStartDirectory);
   EXPECT_EQ(result2.HasFailed(), false);
   EXPECT_FALSE(FileIO::DoesDirectoryExist({baseDir + "/dir1"}));
   EXPECT_FALSE(FileIO::DoesDirectoryExist({baseDir + "/dir2"}));
   EXPECT_FALSE(FileIO::DoesFileExist({baseDir + "/dir1/file.txt"}));
   EXPECT_FALSE(FileIO::DoesDirectoryExist(baseDir)); // BASE DIR IS REMOVED

}

TEST_F(TestFileIO, TestReadAsciiFileContentAsRoot) {

   int previousUID = setfsuid(-1);
   int previousGID = setfsgid(-1);
   ASSERT_EQ(previousUID, 0);
   ASSERT_EQ(previousGID, 0);

   FileIO::SetUserFileSystemAccess("nobody");

   int targetUID = setfsuid(-1);
   int targetGID = setfsgid(-1);
   ASSERT_NE(targetUID, 0);
   ASSERT_NE(targetGID, 0);

   //Open a common root permissioned file without root permissions.
   auto badResult = FileIO::ReadAsciiFileContent("/etc/sysconfig/iptables");
   ASSERT_TRUE(badResult.HasFailed());

   //Open a common root permissioned file.
   auto goodResult = FileIO::ReadAsciiFileContentAsRoot("/etc/sysconfig/iptables");
   EXPECT_FALSE(goodResult.HasFailed());
   EXPECT_TRUE(goodResult.result.size() > 0);
}






TEST_F(TestFileIO, AThousandFiles) {
   using namespace FileIO;
   for (size_t index = 0; index < 1000; ++index) {
      CreateFile(mTestDirectory, std::to_string(index));
   }

   std::vector<std::string> files;
   DirectoryReader::Entry entry;

   DirectoryReader reader(mTestDirectory);
   StopWatch timeToFind;
   entry = reader.Next();
   while (entry.first != FileType::End) {
      ASSERT_NE(entry.first, FileType::Directory);
      ASSERT_NE(entry.first, FileType::Unknown);
      files.push_back(entry.second);
      entry = reader.Next();
   }

   ASSERT_EQ(files.size(), 1000);
   LOG(INFO) << "Time to find 1000 files and save them took: " << timeToFind.ElapsedUs() << " us";

   std::sort(files.begin(), files.end(), [](const std::string& lh, const std::string & rh) {
      return std::stoul(lh) < std::stoul(rh);
   });
   for (size_t index = 0; index < 1000; ++index) {
      EXPECT_EQ(files[index], std::to_string(index));
   }
}


TEST_F(TestFileIO, DirectoryReader_NotExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory + "/_#Does_not+_exist"};
   EXPECT_TRUE(reader.Valid().HasFailed()) << reader.Valid().error;
}

TEST_F(TestFileIO, DirectoryReader_ExistingDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed()) << reader.Valid().error;
}











// An empty directory will only contain "." and ".." which we ignores
TEST_F(TestFileIO, DirectoryReader_NoFilesInDirectory) {
   FileIO::DirectoryReader reader{mTestDirectory};
   EXPECT_FALSE(reader.Valid().HasFailed());

   FileIO::DirectoryReader::Entry fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileIO::FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}

TEST_F(TestFileIO, DirectoryReader_HasFilesInDirectory__AfterReset) {
   using namespace FileIO;

   DirectoryReader reader{mTestDirectory};
   DirectoryReader::Entry fileAndType = reader.Next();

   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");

   // We have already reached the end. This must be reset before reading successfully
   CreateFile(mTestDirectory, "some_file");
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");

   // After the reset we can find the file
   reader.Reset();
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.first, FileType::File);
   EXPECT_EQ(fileAndType.second, "some_file");


   // has reached the end again
   fileAndType = reader.Next();
   EXPECT_EQ(fileAndType.second, "");
   EXPECT_EQ(fileAndType.first, FileType::End);


   CreateSubDirectory("some_directory");
   EXPECT_TRUE(FileIO::DoesDirectoryExist({mTestDirectory + "/some_directory"}));
   reader.Reset();

   fileAndType = reader.Next();
   EXPECT_NE(fileAndType.first, FileType::End);

   std::string filename;
   std::string directoryname;

   for (size_t count = 0; count < 2; ++count) {
      if (fileAndType.first == FileType::Directory) {
         directoryname = fileAndType.second;
         fileAndType = reader.Next();
      } else if (fileAndType.first == FileType::File) {
         filename = fileAndType.second;
         fileAndType = reader.Next();
      } else { 
         std::cout << "got unknown result" << fileAndType.second << std::endl;
      }
   }

   EXPECT_EQ(filename, "some_file");
   EXPECT_EQ(directoryname, "some_directory");
   fileAndType = reader.Next();

   EXPECT_EQ(fileAndType.first, FileType::End);
   EXPECT_EQ(fileAndType.second, "");
}



TEST_F(TestFileIO, Directory_or_bogus_IsNotAMountPoint) {
   auto directory = CreateSubDirectory("A_Directory");
   std::string expectedDirectory  = {mTestDirectory + "/A_Directory"};
   EXPECT_EQ(directory, expectedDirectory);
   EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
   EXPECT_FALSE(FileIO::IsMountPoint(directory).result);
   EXPECT_TRUE(FileIO::IsMountPoint(directory).HasFailed());  
      
   EXPECT_FALSE(FileIO::IsMountPoint("/does_not_exist/ever/").result);
   EXPECT_TRUE(FileIO::IsMountPoint("/does_not_exist/ever/").HasFailed());   

   
   auto file = CreateFile(directory, "test_file");
   EXPECT_TRUE(FileIO::DoesFileExist(file));
   EXPECT_FALSE(FileIO::IsMountPoint(file).result);   
   EXPECT_TRUE(FileIO::IsMountPoint(file).HasFailed());   
}


TEST_F(TestFileIO, SYSTEM__Slash_is_always_A_MountPoint) {
   std::string directory = {"/"};
   EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
   
   Result<bool> status = FileIO::IsMountPoint(directory);
   EXPECT_TRUE(status.result) << status.error;
   EXPECT_FALSE(status.HasFailed()) << status.error;
}

// This could fail if the system is setup with /root as its own partition
// it is fairly uncommon to do that so I keep it as this for now / Kjell
TEST_F(TestFileIO, SYSTEM__root_is_not_A_MountPoint) {
   std::string directory = {"/root"};
   EXPECT_TRUE(FileIO::DoesDirectoryExist(directory));
   
   Result<bool> status = FileIO::IsMountPoint(directory);
   EXPECT_FALSE(status.result) << status.error;
   EXPECT_TRUE(status.HasFailed()) << status.error;
}


TEST_F(TestFileIO, SYSTEM__Dependent_Stuff__MountPoint) {
   
   Result<bool> status = FileIO::IsMountPoint("/").result;
   EXPECT_TRUE(status.result) << status.error;
   EXPECT_FALSE(status.HasFailed()) << status.error;
   
   
   EXPECT_TRUE(FileIO::IsMountPoint("/home").result);
}



// FileIO #files   time
//        63,8841  761 ms
//        994,080  1 sec

// Boost #files   time
//       63,8841  3199 ms
//       985,524  5 sec
TEST_F(TestFileIO, DISABLED_System_Performance_FileIO__vs_Boost) {
   using namespace FileIO;

   DirectoryReader::Entry entry;

   StopWatch timeToFind;

   size_t filecounter = 0;

   std::string path = {"/tmp"};
   DirectoryReader reader(path);
   if (false == reader.Valid().HasFailed()) {
      reader.Next();
      while (entry.first != FileIO::FileType::End) {
         ++filecounter;
      }
      entry = reader.Next();
   }

  
   LOG(INFO) << "FileIO Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec";
   timeToFind.Restart();
   boost::filesystem::path boostPath = path;
   boost::filesystem::directory_iterator end;
   filecounter = 0;
   for (boost::filesystem::directory_iterator dir_iter(boostPath); dir_iter != end; ++dir_iter) {
      if (boost::filesystem::is_regular_file(dir_iter->status())) {
         ++filecounter;
      }
   }
   LOG(INFO)<< "Boost Time to find " << filecounter << "took: " << timeToFind.ElapsedSec() << " sec";
}
