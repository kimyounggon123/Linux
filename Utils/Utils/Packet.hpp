#ifndef _PACKET_H
#define _PACKET_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

#include "Buffers.hpp"






// 패킷 당 작업을 구별하는 flag
enum class PacketType : uint32_t
{
	Default,
	ERROR_TYPE,
	ServerIsClosed,


	LastDummy // Put this the last.
};


// 패킷 작업 성공 여부 및 브로드캐스팅/DB flag
enum class PacketResult : uint32_t
{
	Try,
	Success,
	Fail,

	Broadcast,

	WaitDatabase ,
	DatabaseSuccess,
	DatabaseFail,


	LastDummy // Put this the last.
};


inline bool IsValidFlag(const PacketResult& yourFlag)
{
	return yourFlag == PacketResult::Try || yourFlag == PacketResult::Success || yourFlag == PacketResult::Fail || yourFlag == PacketResult::Broadcast || yourFlag == PacketResult::WaitDatabase
		|| yourFlag == PacketResult::DatabaseSuccess || yourFlag == PacketResult::DatabaseFail;
}
inline bool IsValidFlagUINT(const uint32_t& toChange)
{
	return IsValidFlag(static_cast<PacketResult>(toChange));
}

template <typename T>
inline uint32_t ChangeToUINT(const T& toChange)
{
	return static_cast<uint32_t>(toChange);
}
template <typename T>
inline T ChangeToEnumClass(const uint32_t toChange)
{
	return static_cast<T>(toChange);
}


#pragma pack(push, 1)   // 1바이트 단위로 정렬 시작
struct PacketHeader
{
	uint32_t clientID; // 주로 토큰 값으로 설정.
	uint32_t type;
	uint32_t result;
	uint32_t length;

	PacketHeader(bool networkEndianFlag = true) : clientID(0), type(ChangeToUINT(PacketType::Default)), result(ChangeToUINT(PacketResult::Try)), length(0)
	{
	}
	PacketHeader(uint32_t clientID, uint32_t type, uint32_t result, bool networkEndianFlag = true) : clientID(clientID), type(type), result(result), length(0)
	{
	}
	PacketHeader(const PacketHeader& other) : clientID(other.clientID), type(other.type), result(other.result), length(0)
	{
	}

	void CopyOther(const PacketHeader& other)
	{
		clientID = other.clientID;
		type = other.type;
		result = other.result;
	}
};
#pragma pack(pop)


//template <bool UseEndianChange>
class Packet
{
	// Header information
	PacketHeader header;
	//std::vector<BYTE> data;
	RawDataBuffer buffer;
	void AppendRawMemory(const BYTE* src, const size_t& size);
	void ReadRawMemory(BYTE* dest, const size_t& size, size_t& offset);

public:
	static const uint32_t END_MARK;
	static const uint32_t MAX_SIZE;

	Packet();
	Packet(const Packet& other);

	~Packet();


	void CLEAR_PACKET(bool delete_pk = false); // clear packet data. you can change header

	/// Deep copy
	void CopyOther(const Packet* other);
	void CopyOther(const Packet& other);

	/// Input control methods
	ERROR_CODE PushBytes(const BYTE* src, const size_t& size);
	ERROR_CODE OverlapBytes(const BYTE* src, const size_t& size, size_t& offset); // Push 가 아니라 기존 데이터를 덮어 씌우는 코드입니다 주의.

	template <typename T>
	ERROR_CODE PushNumericData(const T& src);

	// num input methods
	ERROR_CODE PushInt(const int32_t& src);
	ERROR_CODE PushFloat(const float& src);

	// string input methods
	ERROR_CODE PushStringUTF8(const std::string& data);

	/// Read control methods
	ERROR_CODE ReadBytes(BYTE* dest, const size_t& size, size_t& offset);

	template <typename T>
	ERROR_CODE ReadNumericData(T& dest, size_t& offset);

	ERROR_CODE ReadInt(int32_t& dest, size_t& offset);
	ERROR_CODE ReadFloat(float& dest, size_t& offset);
	ERROR_CODE ReadDouble(double& dest, size_t& offset);

	// string input methods
	ERROR_CODE ReadStringUTF8(std::string& dest, size_t& offset);

	ERROR_CODE Serialize(BYTE* sendBuffer, size_t& offset);
	ERROR_CODE Serialize(std::vector<BYTE>& sendBuffer);

	ERROR_CODE Deserialize(const BYTE* recvBuffer, int recvLength, size_t& offset);



	void PrintInformation(const std::string& where = "UNKNOWN");

	/// Getter
	size_t GetContentSize() const { return buffer.GetSize(); }
	size_t GetSerializedSize() { return sizeof(PacketHeader) + buffer.GetSize() + sizeof(uint32_t); }

	uint32_t GetClientID() const { return header.clientID; }

	template <typename T>
	T GetType() const { return static_cast<T>(header.type); }

	template <typename T>
	T GetResult() const { return static_cast<T>(header.result); }

	template <typename T>
	void SetClientID(const T& id) { header.clientID = static_cast<uint32_t>(id); }

	template <typename T>
	void SetType(const T& type) { header.type = static_cast<uint32_t>(type); }

	template <typename T>
	void SetResult(const T& result) { header.result = static_cast<uint32_t>(result); }
};
#endif