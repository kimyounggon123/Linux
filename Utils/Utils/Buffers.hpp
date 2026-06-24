#ifndef BUFFERS_H
#define BUFFERS_H


#include <vector>
#include "utils.h"
class RecvBuffer
{
    static constexpr int RECV_BUFFER_SIZE = 32768; // 32KB

    int read_pos;
    int write_pos;

    BYTE buffer[RECV_BUFFER_SIZE];

public:
    RecvBuffer() : read_pos(0), write_pos(0), buffer{} {}

    void OnWrite(int size);
    void OnRead(int size);

    void Clear();

    // 남은 버퍼 내 정보를 앞으로 당기는 method
    void MoveDataFront();

    char* GetBufferToByte() { return reinterpret_cast<char*>(buffer + read_pos); }

    BYTE* GetWritePtr() { return &buffer[write_pos]; }          // 현재 버퍼 시작 위치
    BYTE* GetReadPtr() { return &buffer[read_pos]; } 

    int GetFreeSpace() const { return RECV_BUFFER_SIZE - write_pos; } // 남은 버퍼 크기
    int GetRemainSize() const { return write_pos - read_pos; }     // 현재 담긴 버퍼양

    bool IsEmpty() const { return GetRemainSize() == 0; }

};



class SendBuffer
{
    static constexpr int SEND_BUFFER_SIZE = 65536; // 64KB
    std::vector<BYTE> buffer;

public:
    SendBuffer()
    {
        buffer.reserve(SEND_BUFFER_SIZE);
    }
    ~SendBuffer()
    {
        Clear();
    }

    const char* GetBufferToCHAR() {return reinterpret_cast<const char*>(buffer.data()); }
    const BYTE* GetBuffer() const { return buffer.data(); }
    std::vector<BYTE>& GetVector() {return buffer;}

    void PushFrontRange(int retval)
    {
        buffer.erase(buffer.begin(), buffer.begin() + retval);   
    }
    void Clear()
    {
        buffer.clear();
    }
    size_t Size() const {return buffer.size();}
};




enum class ERROR_CODE : uint8_t 
{
	NONE,  

	OTHER_ERROR, // default error code

	GET_NULLPTR,
	INCORRECT_SIZE,
	MEMORY_LIMIT,
	EMPTY_CONTAINOR,

	UNDEFINED_FUNCTION, // if you run undefined function
	FUNCTION_RUNNING_FAILED, // get function failed result 

	HAVETO_DELETE_PACKET_HEADER,
	NEED_EXTRA_DATA,		// packet comes here imperfectly
	OPENED_PACKET, // if packet doesn't have end mark;

	CONTAINOR_ERROR,
	WRONG_DATA,

	SUCCESS
};
constexpr bool operator!(ERROR_CODE code)
{
	return code != ERROR_CODE::SUCCESS;
}


#include <arpa/inet.h>
class RawDataBuffer
{
    bool bigEndianFlag;
    std::vector<BYTE> buffer;

    void AppendRawMemory(const BYTE* src, const size_t& size);
	void ReadRawMemory(BYTE* dest, const size_t& size, size_t& offset);

public:
    RawDataBuffer(size_t capacity, bool bigEndianFlag = true): bigEndianFlag(bigEndianFlag)
    {
        buffer.reserve(capacity);
    }

    RawDataBuffer& operator=(const RawDataBuffer& other)
    {
        if (this == &other) return *this; 
        
        if (buffer.capacity() < other.buffer.size()) buffer.reserve(other.buffer.capacity()); 
    
        buffer.clear();
        buffer.assign(other.buffer.begin(), other.buffer.end());

        return *this;
    }

    RawDataBuffer(const RawDataBuffer& other)
    {
        *this = other;
    }

    ~RawDataBuffer()
    {
        Clear();
    }
    
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

	ERROR_CODE ExtractData(std::vector<BYTE>& sendBuffer);
    ERROR_CODE ExtractData(BYTE* sendBuffer, size_t& offset);

    size_t GetSize() const {return buffer.size();}

    void Clear() {buffer.clear();}
};


#endif