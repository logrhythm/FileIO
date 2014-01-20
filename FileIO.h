/* 
 * File:   FileIO.h
 * Author: kjell
 *
 * Created on August 15, 2013, 2:28 PM
 */

#pragma once
#include <string>

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
      : result(output), error(err){}      
      
      /** @return status whether or not the Result contains a failure*/
      bool HasFailed() { return (!error.empty());}
   };
   
   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile); 
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   bool DoesFileExist(const std::string& pathToFile);
}

