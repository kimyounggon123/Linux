#include "DatabaseServer.hpp"
int main()
{
    std::unique_ptr<DatabaseServer> server = std::make_unique<DatabaseServer>(8080, 8);
	ServerAgent* agent = new ServerAgent(std::move(server));
	try
	{
		if (agent == nullptr) throw "Null object";
    	if (!agent->Initialize()) throw "Initialize";
		std::cout<< " ======================== Start ======================== " << std::endl;
    	agent->Start();
		agent->InputCommand();
		agent->Stop();
	}
	catch(const char* msg)
	{
		std::cout << msg << " ERROR" << std::endl;
	}
	if (agent != nullptr) delete agent;
	std::cout << " ======================== End ======================== " << std::endl;
    return 0;
}