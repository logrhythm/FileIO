
#include "ForkerPipe.h"
#include <iostream>
#include <fcntl.h>
#include "libtools/ForkerRequest.pb.h"
#include "czmq.h"
#include <thread>

/**
 * Creates the fifo if not the client
 */
ForkerPipe::ForkerPipe(const std::string clientName, bool client) : mClientName(clientName),
mClient(client) {
   std::string fifo = "/tmp/";

   std::string::size_type start = mClientName.find_first_not_of("/", 0);
   std::string::size_type stop = mClientName.find_first_of("/", start);

   std::vector<std::string> tokens;
   while (std::string::npos != stop || std::string::npos != start) {
      tokens.push_back(mClientName.substr(start, stop - start));
      start = mClientName.find_first_not_of("/", stop);
      stop = mClientName.find_first_of("/", start);
   }
   std::cout << tokens[tokens.size() - 1] << std::endl;
   fifo += tokens[tokens.size() - 1];

   mFifoClient = fifo;
   mFifoClient += ".input.fifo";
   mFifoForker = fifo;
   mFifoForker += ".output.fifo";

   std::string command("/usr/bin/mkfifo ");
   command += mFifoClient;
   system(command.c_str());

   command = ("/usr/bin/mkfifo ");
   command += mFifoForker;
   system(command.c_str());


}

/**
 * The non-client deletes the fifo
 */
ForkerPipe::~ForkerPipe() {

   if (mClient) {
      std::string command("/bin/rm ");
      command += mFifoClient;
      system(command.c_str());
   } else {
      std::string command("/bin/rm ");
      command += mFifoForker;
      system(command.c_str());
   }

}

/**
 * Send a command to the Forker, return stdout of the command pipe
 * 
 * @return if the command ran from the forker
 */
bool ForkerPipe::SendCommand(const std::string& command, const std::vector<std::string> args, std::string& result) {
   result = "";
   if (!mClient) {
      return false;
   }

   protoMsg::ForkerRequest requestProto;
   requestProto.set_command(command);
   int index(0);
   for (const auto& arg : args) {
      requestProto.set_args(index++, arg);
   }
   std::string serialized = requestProto.SerializeAsString();

   if (!SendStringToPipe(serialized)) {
      return false;
   }

   return GetStringFromPipeWithWait(result, 60);
}

bool ForkerPipe::SendStringToPipe(const std::string& serialized) {
   std::string target;
   if (mClient) {
      target = mFifoForker;
   } else {
      target = mFifoClient;
   }
   int pipe = open(target.c_str(), O_WRONLY);
   if (0 > pipe) {
      return false;
   }
   write(pipe, serialized.c_str(), serialized.size());
   close(pipe);
}

bool ForkerPipe::GetStringFromPipeWithWait(std::string& resultString, const int waitInSeconds) {
   std::string target;
   if (mClient) {
      target = mFifoClient;
   } else {
      target = mFifoForker;
   }
   bool success(false);
   fd_set fileDescriptors;
   struct timeval timeout;
   int count(0);
   int returnValue(-1);
   resultString = "";
   int pipe = open(target.c_str(), O_RDONLY|O_NONBLOCK);
   if (0 > pipe) {
      std::this_thread::sleep_for(std::chrono::seconds(waitInSeconds));
      return false;
   }
   while (count++ < waitInSeconds && !zctx_interrupted && returnValue < 0) {
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      FD_ZERO(&fileDescriptors);
      FD_SET(pipe, &fileDescriptors);
      returnValue = select(pipe + 1, &fileDescriptors, NULL, NULL, &timeout);
   }
   if (returnValue >= 0) {
      char characters[32];
      size_t sizeRead(0);
      do {
         sizeRead = read(pipe, characters, 32);
         for (int i = 0; i < sizeRead; i++) {
            resultString += characters[i];
         }
      } while (sizeRead > 0);
      success = true;
   }
   close(pipe);
   return success;

}

/**
 * Select for timeout on the pipe, return true and valid info in command/args if one was seen
 */
bool ForkerPipe::GetCommand(std::string& command, std::vector<std::string> args, const int timeout) {
   if (mClient) {
      return false;
   }
   bool success(false);
   std::string serialized;
   if (GetStringFromPipeWithWait(serialized, timeout)) {
      protoMsg::ForkerRequest requestProto;
      requestProto.ParseFromString(serialized);
      if (requestProto.has_command()) {
         success = true;
         command = requestProto.command();
      }

      for (int i = 0; i < requestProto.args_size(); i++) {
         args.push_back(requestProto.args(i));
      }
   }

   return success;
}

