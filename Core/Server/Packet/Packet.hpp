#ifndef _PACKET_H
#define _PACKET_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

#include "../../Utils/utils.h"
#include "../../Utils/Buffer/Buffers.hpp"

// 패킷의 성질 구별
enum class Reliability : uint8_t
{
    Unreliable,        // 보장 없음
    Reliable,           // 순서만 보장
    ReliableOrdered    // 순서 + 재전송
};

// 패킷 당 작업을 구별하는 flag
enum class PacketChannel : uint8_t
{
	GENERAL, 
	LAST_DUMMY
};
enum class PacketPriority : uint8_t
{
    Critical,   // 로그인, 결제
    Normal,     // 채팅
    Low          // 위치 업데이트
};
enum class PacketType : uint32_t
{
	Default,
	ERROR_TYPE,
	Test,
	HelloNewClient,
	ByeClient,

	LAST_DUMMY // Put this the last.
};

// 패킷 작업 성공 여부 및 브로드캐스팅/DB flag
enum class PacketResult : uint32_t
{
	Try,
	Success,

	// 실패의 경우 실패 요인을 code로 작성.
	SERVER_IS_CLOSED,

	DELETED_PACKET,
	OTHER_ERROR,

	PARAMETER_ERROR,
	CALL_NULL_METHOD,
	RESOURCE_FAMINE,
	INVALID_CLIENT,
	BANNED,

	LAST_DUMMY // Put this the last.
};

#pragma pack(push, 1)   // 1바이트 단위로 정렬 시작
struct PacketHeader
{
	uint8_t goal;

	uint32_t clientID; // 주로 토큰 값으로 설정.
	uint32_t type;
	uint32_t result;
	uint32_t length;
	uint32_t sequence_count;
	
	PacketHeader() : 
		goal(ChangeToUINT(PacketChannel::GENERAL)),
		clientID(0), 
		type(ChangeToUINT(PacketType::Default)), result(ChangeToUINT(PacketResult::Try)), 
		length(0), sequence_count(0)
	{}
	PacketHeader(uint8_t processCategory, uint8_t goal,
		uint32_t clientID, uint32_t type, uint32_t result) :
		goal(goal),
	 	clientID(clientID), type(type), result(result), length(0), 
		sequence_count(0)
	{}
	PacketHeader(const PacketHeader& other): 
		goal(other.goal),
		clientID(other.clientID), type(other.type), result(other.result), length(0),
		sequence_count(other.sequence_count)
	{}

	void CopyOther(const PacketHeader& other)
	{
		goal = other.goal;

		clientID = other.clientID;
		type = other.type;
		result = other.result;
		length = other.length;
		sequence_count = other.sequence_count;
	}

	PacketHeader ChangeThisToNetworkEndian()
	{
		PacketHeader netHeader;
		netHeader.goal = goal;

		netHeader.clientID = htonl(clientID);
		netHeader.type = htonl(type);
		netHeader.result = htonl(result);
		netHeader.length = htonl(length);
		netHeader.sequence_count = htonl(sequence_count);

		return netHeader;
	}

	void ChangeThisToHostEndian(const PacketHeader& netHeader)
	{
		goal = netHeader.goal;
		clientID = ntohl(netHeader.clientID);
		type = ntohl(netHeader.type);
		result = ntohl(netHeader.result);
		length = ntohl(netHeader.length);
		sequence_count = ntohl(netHeader.sequence_count);
	}

};
#pragma pack(pop)


//template <bool UseEndianChange>
class Packet
{
	// Header information
	PacketHeader header;
	RawDataBuffer buffer;

public:
	static const uint32_t END_MARK;
	static const uint32_t MAX_SIZE;

	Packet(): header(), buffer(MAX_SIZE)
	{}
	Packet(const Packet& other): header(other.header), buffer(MAX_SIZE)
	{
		buffer = other.buffer;
	}
	~Packet()
	{
		buffer.Clear();
	}

	void ClearBuffer()
	{
		buffer.Clear();
		header.length = 0;
	}
	void CLEAR_PACKET()
	{
		ClearBuffer();
		SetType(ChangeToUINT(PacketType::ERROR_TYPE));
		SetResult(PacketResult::DELETED_PACKET);
	}

	/// Deep copy
	void CopyOther(const Packet& other)
	{
		header.CopyOther(other.header);
		buffer = other.buffer;
	}
	void CopyOther(const Packet* other)
	{
		if (other == nullptr) return;
		CopyOther(*other);
	}

	/// Input control methods
	ERROR_CODE PushBytes(const BYTE* src, const size_t& size) {return buffer.PushBytes(src, size);}
	// Push 가 아니라 기존 데이터를 덮어 씌우는 코드입니다 주의.
	ERROR_CODE OverlapBytes(const BYTE* src, const size_t& size, size_t& offset) {return buffer.OverlapBytes(src, size, offset);} 

	// num input methods
	ERROR_CODE PushInt8(const uint8_t& src) {return buffer.PushInt8(src);}
    ERROR_CODE PushInt16(const uint16_t& src) {return buffer.PushInt16(src);}
    ERROR_CODE PushInt32(const uint32_t& src) {return buffer.PushInt32(src);}
	ERROR_CODE PushFloat(const float& src)	{return buffer.PushFloat(src);}

	// string input methods
	ERROR_CODE PushStringUTF8(const std::string& data) {return buffer.PushStringUTF8(data);}

	/// Read control methods
	ERROR_CODE ReadBytes(BYTE* dest, const size_t& size) {return buffer.ReadBytes(dest, size);}

	ERROR_CODE ReadInt8(uint8_t& dest) {return buffer.ReadInt8(dest);}
    ERROR_CODE ReadInt16(uint16_t& dest) {return buffer.ReadInt16(dest);}
    ERROR_CODE ReadInt32(uint32_t& dest) {return buffer.ReadInt32(dest);}
	ERROR_CODE ReadFloat(float& dest) {return buffer.ReadFloat(dest);}

	// string input methods
	ERROR_CODE ReadStringUTF8(std::string& dest) {return buffer.ReadStringUTF8(dest);}

	Packet& operator>>(uint8_t& data) { ReadInt8(data); return *this; }
    Packet& operator>>(uint16_t& data) { ReadInt16(data); return *this; }
    Packet& operator>>(uint32_t& data) { ReadInt32(data); return *this; }
    Packet& operator>>(float& data) { ReadFloat(data); return *this; }
    Packet& operator>>(std::string& data) { ReadStringUTF8(data); return *this; }


	ERROR_CODE Serialize(BYTE* sendBuffer);
	ERROR_CODE Serialize(std::vector<BYTE>& sendBuffer);
	ERROR_CODE Deserialize(const BYTE* recvBuffer, int remainLength);


	void PrintInformation(const std::string& where = "UNKNOWN");

	/// Getter
	size_t GetContentSize() const { return buffer.GetSize(); }
	size_t GetSerializedSize() { return sizeof(PacketHeader) + buffer.GetSize() + sizeof(uint32_t); }


	//uint8_t processCategory;
	//uint8_t goal;
	PacketChannel GetGoal() const {return static_cast<PacketChannel>(header.goal);}

	uint32_t GetClientID() const { return header.clientID; }

	template <typename T>
	T GetType() const { return static_cast<T>(header.type); }
	uint32_t GetTypeUINT() const {return header.type;}

	PacketResult GetResult() const { return static_cast<PacketResult>(header.result); }
	uint32_t GetResultUINT() const {return header.result;}

	void SetGoal(const PacketChannel& result) { header.goal = static_cast<uint8_t>(result); }

	void SetClientID(const uint32_t& id) { header.clientID = id; }

	template <typename T>
	void SetType(const T& type) { header.type = static_cast<uint32_t>(type); }
	void SetResult(const PacketResult& result) { header.result = static_cast<uint32_t>(result); }


	bool IsValid() const {return buffer.IsValid();}
	size_t GetCurrOffset() const {return buffer.GetCurrOffset();}
	void SetOffset(size_t offset) {buffer.SetOffset(offset);}
};
#endif