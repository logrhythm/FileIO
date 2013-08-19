/* 
 * File:   FileIO.cpp
 * Author: kjell
 * 
 * Created on August 15, 2013, 2:28 PM
 */

#include "FileIO.h"
#include <fstream>
#include <g2log.hpp>
namespace FileIO {
   /**
    * Reads content of Ascii file
    * @param pathToFile to read
    * @return Result<std::string> all the content of the file, and/or an error string 
    *         if something went wrong 
    */
Result<std::string> ReadAsciiFileContent(const std::string& pathToFile) {
   std::ifstream in(pathToFile, std::ios::in);
   if (!in) {
      std::string error{"Cannot read-open file: "};
      error.append(pathToFile);
      return Result<std::string>{{}, error};
   }

   std::string contents;
   in.seekg(0, std::ios::end);
   auto end = in.tellg();

   // Attempt to read it the fastest way possible
   if(-1 != end) {
      contents.resize(end);
      in.seekg(0, std::ios::beg);
      in.read(&contents[0], contents.size());
      in.close();
      return Result<std::string>{contents};
   }
   
   // Could not calculate with ifstream::tellg(). Is it a RAM file? 
   // Fallback solution to slower iteratator approach
   contents.assign((std::istreambuf_iterator<char>(in)),  
                   (std::istreambuf_iterator<char>()  ) );
   return Result<std::string>{contents};
}

   /**
    * Write ascii content to file
    * @param pathToFile to write
    * @param content to write to file
    * @return Result<bool> result if operation whent OK, if it did not the Result<bool>::error string 
    *         contains the error message
    */
Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content) {
   std::ofstream out(pathToFile, std::ios::out | std::ios::trunc);
   if (!out) {
      std::string error{"Cannot write-open file: "};
      error.append(pathToFile);
      return Result<bool>{false, error};
   }
   
   out << content;
   out.close();
   return Result<bool>{true};
}


}