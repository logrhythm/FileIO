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
   std::string SendCommand(const std::string& command, const std::vector<std::string> args);
   bool GetCommand(std::string& command,std::vector<std::string> args,const int timeout);
   std::string RunCommand(std::string& command,std::vector<std::string> args);
protected:
   
private:
   std::string mClientName;
   bool mClient;
   std::string mFifo;
};

