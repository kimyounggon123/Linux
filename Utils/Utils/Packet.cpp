#include "Packet.hpp"

const uint32_t Packet::END_MARK = 0xffffffff;
const uint32_t Packet::MAX_SIZE = BUFFERSIZE;

Packet::Packet(): header(0, ChangeToUINT(PacketType::Default), ChangeToUINT(PacketResult::Try)), buffer(MAX_SIZE)
{
}
Packet::Packet(const Packet& other): header(other.header), buffer(MAX_SIZE)
{
	buffer = other.buffer;
}
Packet::~Packet()
{
	buffer.Clear();
}


void Packet::CLEAR_PACKET(bool delete_pk)
{
	buffer.Clear();
	header.length = 0;
	if (delete_pk) SetType(ChangeToUINT(PacketType::Default));
}
/// Deep copy
void Packet::CopyOther(const Packet* other)
{
	if (other == nullptr) return;
	CopyOther(*other);
}
void Packet::CopyOther(const Packet& other)
{
	header.CopyOther(other.header);
	buffer = other.buffer;
}

/// Input control methods
// void Packet::AppendRawMemory(const BYTE* src, const size_t& size)
// {
// 	data.insert(data.end(), src, src + size);
// 	header.length = static_cast<uint32_t>(data.size());
// }

ERROR_CODE Packet::PushBytes(const BYTE* src, const size_t& size)
{
	if (src == nullptr) return ERROR_CODE::GET_NULLPTR;
	if (size < 1 || header.length +  size > MAX_SIZE) return ERROR_CODE::INCORRECT_SIZE;
	return buffer.PushBytes(src, size);
}
ERROR_CODE Packet::OverlapBytes(const BYTE* src, const size_t& size, size_t& offset)
{
	if (size + offset > header.length) return ERROR_CODE::INCORRECT_SIZE;
	return buffer.OverlapBytes(src, size, offset);
}	

template <typename T>
ERROR_CODE Packet::PushNumericData(const T& src)
{
	return buffer.PushNumericData(src);
}


// num input methods
ERROR_CODE Packet::PushInt(const int32_t& src)
{
	return PushNumericData(src);
}
ERROR_CODE Packet::PushFloat(const float& src)
{
	return PushNumericData(src);
}

// string input methods
ERROR_CODE Packet::PushStringUTF8(const std::string& src)
{
	int str_len = static_cast<int>(src.size());
	if (header.length + sizeof(int) + str_len > MAX_SIZE) return ERROR_CODE::INCORRECT_SIZE;

	//ERROR_CODE code = InputNumericData(static_cast<BYTE>(1), offset); 
	//if (code != ERROR_CODE::SUCCESS) return code;

	ERROR_CODE code = PushNumericData(str_len);
	if (code != ERROR_CODE::SUCCESS) return code;
	code = PushBytes(reinterpret_cast<const BYTE*>(src.data()), str_len); // copy string data

	return ERROR_CODE::SUCCESS;
}

/// Read control methods
// void Packet::ReadRawMemory(BYTE* dest, const size_t& size, size_t& offset)
// {
// 	memcpy(dest, &data[offset], size);
// 	offset += size;
// }


ERROR_CODE Packet::ReadBytes(BYTE* dest, const size_t& size, size_t& offset)
{
	return buffer.ReadBytes(dest, size, offset);
}

template <typename T>
ERROR_CODE Packet::ReadNumericData(T& dest, size_t& offset)
{
	return buffer.ReadNumericData(dest, offset);
}


ERROR_CODE Packet::ReadInt(int32_t& dest, size_t& offset)
{
	return buffer.ReadInt(dest, offset);
}
ERROR_CODE Packet::ReadFloat(float& dest, size_t& offset)
{
	return buffer.ReadFloat(dest, offset);
}
ERROR_CODE Packet::ReadDouble(double& dest, size_t& offset)
{
	return buffer.ReadDouble(dest, offset);
}

// string input methods
ERROR_CODE Packet::ReadStringUTF8(std::string& dest, size_t& offset)
{
	return buffer.ReadStringUTF8(dest, offset);
}

ERROR_CODE Packet::Serialize(BYTE* sendBuffer, size_t& offset)
{
	if (sendBuffer == nullptr) return ERROR_CODE::GET_NULLPTR;
	header.length = static_cast<uint32_t>(buffer.GetSize());

	// 헤더 넣기
	PacketHeader netHeader;
	netHeader.clientID = htonl(header.clientID);
	netHeader.type = htonl(header.type);
	netHeader.result = htonl(header.result);
	netHeader.length = htonl(header.length);

	// 한번에 넣기
	memcpy(sendBuffer + offset, &netHeader, sizeof(PacketHeader));
	offset += sizeof(PacketHeader);

	ERROR_CODE code = buffer.ExtractData(sendBuffer, offset);
	if (!code) return code;	

	memcpy(sendBuffer + offset, &END_MARK, sizeof(END_MARK));
	offset += sizeof(END_MARK);

	return ERROR_CODE::SUCCESS;
}

ERROR_CODE Packet::Serialize(std::vector<BYTE>& sendBuffer)
{
	PacketHeader netHeader;
	netHeader.clientID = htonl(header.clientID);
	netHeader.type = htonl(header.type);
	netHeader.result = htonl(header.result);
	netHeader.length = htonl(header.length);
	
	BYTE* headerPtr = reinterpret_cast<BYTE*>(&netHeader);
    sendBuffer.insert(sendBuffer.end(), headerPtr, headerPtr + sizeof(PacketHeader));

	ERROR_CODE code = buffer.ExtractData(sendBuffer);
	if (!code) return code;	

	uint32_t netEndMark = END_MARK;
	BYTE* endmarkPTR = reinterpret_cast<BYTE*>(&netEndMark);
	sendBuffer.insert(sendBuffer.end(), endmarkPTR, endmarkPTR + sizeof(uint32_t));

	return ERROR_CODE::SUCCESS;
}

ERROR_CODE Packet::Deserialize(const BYTE* recvBuffer, int recvLength, size_t& offset)
{
	if (!recvBuffer) return ERROR_CODE::GET_NULLPTR;
	size_t localOffset = offset; // local 복사

	// 헤더 데이터가 완전이 수신되었는가?
	if (localOffset + sizeof(PacketHeader) > recvLength)
	{
		return ERROR_CODE::NEED_EXTRA_DATA;
	}

	PacketHeader netHeader;

	// 버퍼에서 구조체 크기만큼 한 번에 읽어옴
	memcpy(&netHeader, recvBuffer + localOffset, sizeof(PacketHeader));
	localOffset += sizeof(PacketHeader);

	// 읽어온 네트워크 바이트를 호스트 엔디안으로 변환하여 멤버 변수에 저장
	header.clientID = ntohl(netHeader.clientID);
	header.type = ntohl(netHeader.type);
	header.result = ntohl(netHeader.result);
	header.length = ntohl(netHeader.length);

	// 헤더 length가 유효한가?
	if (header.length > MAX_SIZE)
	{
		return ERROR_CODE::INCORRECT_SIZE;
	}

	// 데이터가 확실하게 다 왔는가?
	size_t total_required_size = localOffset + header.length + sizeof(END_MARK);
	if (total_required_size > static_cast<size_t>(recvLength))
	{
		return ERROR_CODE::NEED_EXTRA_DATA;
	}

	ERROR_CODE code = buffer.PushBytes(recvBuffer + localOffset,  header.length);
	if (!code) return code;
	localOffset += header.length;

	unsigned int received_end_mark = 0;
	memcpy(&received_end_mark, recvBuffer + localOffset, sizeof(received_end_mark));
	if (received_end_mark != END_MARK)
	{
		//printf("received_end_mark: 0x%08X\n", received_end_mark); // hex 출력
		//printf("header type: %d\n", header.type);
		//printf("header length: %d\n", header.length);
		return ERROR_CODE::OPENED_PACKET;
	}

	localOffset += sizeof(END_MARK);

	// 성공적으로 읽었으면 offset을 실제로 증가시킴
	offset = localOffset;
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