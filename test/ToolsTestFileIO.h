#pragma once
#include "gtest/gtest.h"
#include <cstdlib>
#include <string>
#include "FileIO.h"
#include <iostream>


class TestFileIO : public ::testing::Test {
public:
   TestFileIO() : mTestDirectory("/tmp/TempDirectoryTestOfFileIO") {}
   virtual ~TestFileIO() = default;

   virtual std::string CreateSubDirectory(const std::string& directory,  const std::string& startPath = {}) {
       std::string path{startPath};
       
       if (startPath.empty()) {
           path = mTestDirectory;
      }
       
      if ('/' != startPath.back()) {
          path.append("/");
      }
       
      std::string mkTestSubDir{"mkdir -p " + path + directory};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }
      return {path + directory};
   }
   
   

   virtual std::string CreateFile(const std::string& directory, const std::string& filename) {
      std::string mkTestSubDir{"touch " + directory + "/" + filename};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }
      return {directory + "/" + filename};
   }
    

protected:

   virtual void SetUp() {
      std::string mkTestdir{"mkdir -p " + mTestDirectory};
      EXPECT_EQ(0, system(mkTestdir.c_str()));
   }

   virtual void TearDown() {
      std::string removeDir{"rm -rf " + mTestDirectory};
      EXPECT_EQ(0, system(removeDir.c_str()));
   }
  
   std::string CreateTestDirectoryAndFiles(const std::vector<std::string>& filenamesToTouch, const std::string& newDirName);
   void VerifyDirectoryContents(const std::vector<std::string>& filenamesToVerify, const Result<std::vector<std::string>>& dirContentsResult);

   const std::string mTestDirectory;
};
