/* 
 * File:   Result.h
 * Author: kjell
 *
 * https://github.com/weberr13/FileIO
 * Created on April 21
 */
#pragma once
#include <string>


/**
 * Example Usage: 
 * return Result<bool> success{true};
 * or in case of a failure
 * return Result<bool> failure{false, error};
 */
template<typename T> struct Result {
   const T result;
   const std::string error;

   /**
    * Result of an operation. 
    * @param output whatever the expected output would be
    * @param err error message to the client, default is empty which means successful operation
    */
   Result(T output, const std::string& err)
   : result{output}, error{err} {
   }

   Result(T output) : result{output}, error{""}{
   }

   Result() = delete;
   Result(const Result&) = default;               // Copy constructor
   Result(Result&&) = default;                    // Move constructor
   Result& operator=(const Result&) & = default;  // Copy assignment operator
   Result& operator=(Result&&) & = default;       // Move assignment operator

   ~Result() = default;


   /** @return status whether or not the Result contains a failure*/
   bool HasFailed() const {
      return (!error.empty());
   }
   
   /// convenience function @return status whether or not the Result was a success 
   bool HasSuccess() const {
      return (error.empty());
   }

};
