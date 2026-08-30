#include "Packet.hpp"

const uint32_t Packet::END_MARK = 0xffffffff;
const uint32_t Packet::MAX_SIZE = BUFFERSIZE;



ERROR_CODE Packet::Serialize(BYTE* sendBuffer)
{
	if (sendBuffer == nullptr) return ERROR_CODE::GET_NULLPTR;
	header.length = static_cast<uint32_t>(buffer.GetSize());
	size_t offset = 0;

	// 헤더 넣기
	PacketHeader netHeader = header.ChangeThisToNetworkEndian();

	// 한번에 넣기
	memcpy(sendBuffer + offset, &netHeader, sizeof(PacketHeader));
	offset += sizeof(PacketHeader);

	ERROR_CODE code = buffer.ExtractData(sendBuffer + offset);

	memcpy(sendBuffer + offset, &END_MARK, sizeof(END_MARK));
	offset += sizeof(END_MARK);

	//std::cout << offset << std::endl;
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE Packet::Serialize(std::vector<BYTE>& sendBuffer)
{
	header.length = static_cast<uint32_t>(buffer.GetSize());

	PacketHeader netHeader = header.ChangeThisToNetworkEndian();
	
	BYTE* headerPtr = reinterpret_cast<BYTE*>(&netHeader);
    sendBuffer.insert(sendBuffer.end(), headerPtr, headerPtr + sizeof(PacketHeader));

	ERROR_CODE code = buffer.ExtractData(sendBuffer);

	uint32_t netEndMark = END_MARK;
	BYTE* endmarkPTR = reinterpret_cast<BYTE*>(&netEndMark);
	sendBuffer.insert(sendBuffer.end(), endmarkPTR, endmarkPTR + sizeof(uint32_t));

	//std::cout << sendBuffer.size() << std::endl;
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE Packet::Deserialize(const BYTE* recvBuffer, int remainLength)
{
	if (!recvBuffer) return ERROR_CODE::GET_NULLPTR;

	// 헤더 데이터가 완전이 수신되었는가?
	if (sizeof(PacketHeader) > remainLength)
	{
		return ERROR_CODE::NEED_EXTRA_DATA;
	}

	PacketHeader netHeader;
	size_t localOffset = 0; // local 복사

	// 버퍼에서 구조체 크기만큼 한 번에 읽어옴
	memcpy(&netHeader, recvBuffer, sizeof(PacketHeader));
	localOffset += sizeof(PacketHeader);

	// 읽어온 네트워크 바이트를 호스트 엔디안으로 변환하여 멤버 변수에 저장
	header.ChangeThisToHostEndian(netHeader);
	
	// 헤더 length가 유효한가?
	if (header.length > MAX_SIZE)
	{
		return ERROR_CODE::INCORRECT_SIZE;
	}

	// 데이터가 확실하게 다 왔는가?
	size_t total_required_size = localOffset + header.length + sizeof(END_MARK);
	if (total_required_size > static_cast<size_t>(remainLength))
	{
		return ERROR_CODE::NEED_EXTRA_DATA;
	}

	ERROR_CODE code = buffer.PushBytes(recvBuffer + localOffset, header.length);
	if (!code) return code;
	localOffset += header.length;

	unsigned int received_end_mark = 0;
	memcpy(&received_end_mark, recvBuffer + localOffset, sizeof(received_end_mark));
	if (received_end_mark != END_MARK)
	{
		printf("received_end_mark: 0x%08X\n", received_end_mark); // hex 출력
		return ERROR_CODE::OPENED_PACKET;
	}

	localOffset += sizeof(END_MARK);
	return ERROR_CODE::SUCCESS;
}

void Packet::PrintInformation(const std::string& where)
{
	printf("\nPacket in[%s]\n", where.c_str());
	printf("Packet Type: %d [0x%08X]\n", header.type, header.type);
	printf("Packet result: %d [0x%08X]\n", header.result, header.result);
	printf("Packet Data (Length: %d): \n", header.length);

	/*
	for (auto it = data.cbegin(); it != data.cend(); it++)
	{
		printf("%02X ", *it);
	}
	*/

	
	//for (size_t i = 0; i < header.length; ++i) printf("%02X ", data[i]);
	printf("\n");
}