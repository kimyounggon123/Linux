#include "SignServer.hpp"

int main() {

	std::unique_ptr<SignServer> server = std::make_unique<SignServer>(8080, 8);
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
	// Packet pk;

	// pk.PushStringUTF8("hello world!!!!");
	// pk.PushInt(2);
	// pk.PushFloat(3.24f);
	// pk.PrintInformation("WQEw");

	// BYTE send_buffer[1042];
	// size_t offset = 0;
	// ERROR_CODE err = pk.Serialize(send_buffer, offset);
	// int recv_length = offset;

	// const char* sendThis = reinterpret_cast<const char*>(send_buffer);
	// const BYTE* recvThis = reinterpret_cast<const BYTE*>(sendThis);

	// Packet temp;
	// offset = 0;
	// std::string hello;
	// temp.Deserialize(recvThis, recv_length - 3, offset);

	// offset = 0;
	// int32_t qwewqe = 0;
	// float test = 0.0f;

	// temp.PrintInformation("WQEw");

	// temp.ReadStringUTF8(hello, offset);
	// ERROR_CODE code = temp.ReadInt(qwewqe, offset);
	// if (!code) std::cout << "ERROR: " << ChangeToUINT(code) << std::endl;
	// code = temp.ReadFloat(test, offset);
	// if (!code) std::cout << "ERROR: " << ChangeToUINT(code) << std::endl;

	// std::cout << hello << ' ' <<  qwewqe << ' ' << test << std::endl;

    return 0;
}