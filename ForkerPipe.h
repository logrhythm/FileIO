#pragma once
#include <vector>
#include <string>
/**
 * A class that interacts with the process forker via a named pipe
 */

class ForkerPipe {
public:
   ForkerPipe(const std::string clientName, bool client=true);
   ~ForkerPipe();
   bool GetCommand(std::string& command,std::vector<std::string> args,const int timeout);
   bool SendCommand(const std::string& command, const std::vector<std::string> args, std::string& result);
protected:
   bool GetStringFromPipeWithWait(std::string& resultString,const int waitInSeconds);
   bool SendStringToPipe(const std::string& serialized);
private:
   std::string mClientName;
   bool mClient;
   std::string mFifoClient;
   std::string mFifoForker;
};

