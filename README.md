FileIO
======

Is a collection of file access and directory traversal utilities that was written to be a fast, easy to use and non-exception throwing library. 

The major areas it deals with are:

*_Directory traversal and file access actions_*: 
The _DirectoryReader_ and _FileSystemWalker_ are efficient for traversing directory structures and accessing files with ad-hoc actions on those files. The code was written to be efficient, fast and easy to use. 

Errors are handled through return types and embedded error messages with the [[result]](https://github.com/LogRhythm/FileIO/blob/master/src/Result.h).  In an error-prone scenario the FileIO execution is significantly faster than solutions that utilize exceptions. 


**File Access** 
The file access code was written to avoid rewriting boiler plate code that is easy to get wrong. It covers areas such as file reading and writing, changing file ownership, deleting files and directories and moving of files. Many of these functionalities involve low-level C or C++ APIs that are awkward to use and prone to having bugs. 

The **API** is accessible in the [[src]](https://github.com/LogRhythm/FileIO/tree/master/src) directory. Please read the header files together with [[test cases and example usage]](https://github.com/LogRhythm/FileIO/tree/master/test).

