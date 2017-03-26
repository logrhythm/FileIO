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

## Test Suite Requirements
1. [g3log](https://github.com/KjellKod/g3log)
2. [g3sinks](https://github.com/KjellKod/g3sinks)
3. [StopWatch](https://github.com/LogRhythm/StopWatch)
4. [boost](http://www.boost.org/doc/libs/1_63_0/index.html) 
For installing boost please follow their [instructions](http://www.boost.org/doc/libs/1_63_0/more/getting_started/unix-variants.html). 


## Build and Install
Example of installation
```
cd FileIO
cd 3rdparty
unzip gtest-1.7.0.zip
cd ..
mkdir build
cd build
cmake ..
make -j
```

### Executing the unit tests
```
./UnitTestRunneer
```

### Installing
```
sudo make install
```

Alternative on Debian systems
```
make package
sudo dpkg -i FileIO-<package_version>Linux.deb
```