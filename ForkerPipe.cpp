
#include "ForkerPipe.h"
#include <iostream>
/**
 * Creates the fifo if not the client
 */
ForkerPipe::ForkerPipe(const std::string clientName, bool client) : mClientName(clientName),
mClient(client) {
   mFifo = "/tmp/";

   std::string::size_type start = mClientName.find_first_not_of("/", 0);
   std::string::size_type stop = mClientName.find_first_of("/", start);

   std::vector<std::string> tokens;
   while (std::string::npos != stop || std::string::npos != start) {
      tokens.push_back(mClientName.substr(start, stop - start));
      start = mClientName.find_first_not_of("/", stop);
      stop = mClientName.find_first_of("/", start);
   }
   std::cout << tokens[tokens.size()-1] << std::endl;
   mFifo += tokens[tokens.size()-1];
   mFifo += ".fifo";
   std::cout << mFifo << std::endl;
   if (!client) {
      std::string command("/usr/bin/mkfifo ");
      command += mFifo;
      system(command.c_str());
   }

}

/**
 * The non-client deletes the fifo
 */
ForkerPipe::~ForkerPipe() {
   if (!mClient) {
      std::cout << "removing fifo " << mFifo << std::endl;
      std::string command("/bin/rm ");
      command += mFifo;
      system(command.c_str());
   }
}

/**
 * Send a command to the Forker, return stdout of the command pipe
 */
std::string ForkerPipe::SendCommand(const std::string& command, const std::vector<std::string> args) {
   if (!mClient) {
      return "";
   }
}

/**
 * Select for timeout on the pipe, return true and valid info in command/args if one was seen
 */
bool ForkerPipe::GetCommand(std::string& command, std::vector<std::string> args, const int timeout) {
   if (mClient) {
      return false;
   }
}

/**
 * Run a command, return the stdout of the command
 */
std::string ForkerPipe::RunCommand(std::string& command, std::vector<std::string> args) {
   if (mClient) {
      return "";
   }
}
