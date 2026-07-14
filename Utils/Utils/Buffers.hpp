#ifndef BUFFERS_H
#define BUFFERS_H


#include <atomic>
#include <vector>
#include "utils.h"

class RecvBuffer
{
    static constexpr int RECV_BUFFER_SIZE = 32768; // 32KB

    int read_pos;
    int write_pos;

    BYTE buffer[RECV_BUFFER_SIZE];

public:
    RecvBuffer() : read_pos(0), write_pos(0),
        buffer{} 
    {}

    void OnWrite(int size);
    void OnRead(int size);

    void Clear();

    // 남은 버퍼 내 정보를 앞으로 당기는 method
    void MoveDataFront();

    char* GetBufferToRead() { return reinterpret_cast<char*>(buffer + write_pos); }

    BYTE* GetWritePtr() { return &buffer[write_pos]; }          // 현재 버퍼 시작 위치
    BYTE* GetReadPtr() { return &buffer[read_pos]; } 

    int GetVoidSpace() const { return RECV_BUFFER_SIZE - write_pos; } // 남은 버퍼 크기
    int GetCurrDataSize() const { return write_pos - read_pos; }     // 현재 담긴 버퍼양

    bool IsEmpty() const { return GetCurrDataSize() == 0; }
};


class SendBuffer
{
    static constexpr int SEND_BUFFER_SIZE = 65536; // 64KB
    std::vector<BYTE> buffer;


    std::atomic_bool isSending;
public:
    SendBuffer(): isSending(false)
    {
        buffer.reserve(SEND_BUFFER_SIZE);
    }
    ~SendBuffer()
    {
        Clear();
    }

    const char* GetBufferToSend() {return reinterpret_cast<const char*>(buffer.data()); }
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

    void SetSending(bool flag) {isSending.store(flag);}
    std::atomic_bool& IsSending() {return isSending;}
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
    bool m_isValid;
    bool bigEndianFlag;

    size_t m_offset;
    std::vector<BYTE> buffer;

    ERROR_CODE AppendRawMemory(const BYTE* src, const size_t& size);
	ERROR_CODE ReadRawMemory(BYTE* dest, const size_t& size);

    uint8_t  SwapIfNeeded(uint8_t  val) { return val; }
    uint16_t SwapIfNeeded(uint16_t val) { return bigEndianFlag ? htons(val) : val; }
    uint32_t SwapIfNeeded(uint32_t val) { return bigEndianFlag ? htonl(val) : val; }

    bool CanReadThisPart(const size_t readLength)
    {
        m_isValid = m_isValid && (m_offset + readLength <= buffer.size());
        return m_isValid;
    }

    //uint64_t SwapIfNeeded(uint64_t val) { return bigEndianFlag ? htonll(val) : val; }
public:
    RawDataBuffer(size_t capacity, bool bigEndianFlag = true): 
        m_isValid(true),
        bigEndianFlag(bigEndianFlag), m_offset(0)
    {
        buffer.reserve(capacity);
    }

    RawDataBuffer& operator=(const RawDataBuffer& other)
    {
        if (this == &other) return *this; 
        
        if (buffer.capacity() < other.buffer.size()) buffer.reserve(other.buffer.capacity()); 
        buffer.clear();
        buffer.assign(other.buffer.begin(), other.buffer.end());

        m_offset = other.m_offset;
        m_isValid = other.m_isValid;

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

	// num input methods
	ERROR_CODE PushInt8(const uint8_t& src);
    ERROR_CODE PushInt16(const uint16_t& src);
    ERROR_CODE PushInt32(const uint32_t& src);
	ERROR_CODE PushFloat(const float& src);
    

	// string input methods
	ERROR_CODE PushStringUTF8(const std::string& data);

	/// Read control methods
	ERROR_CODE ReadBytes(BYTE* dest, const size_t& size);

	ERROR_CODE ReadInt8(uint8_t& dest);
    ERROR_CODE ReadInt16(uint16_t& dest);
    ERROR_CODE ReadInt32(uint32_t& dest);
	ERROR_CODE ReadFloat(float& dest);

    // string input methods
	ERROR_CODE ReadStringUTF8(std::string& dest);

    // Util operator
    RawDataBuffer& operator>>(uint8_t& data) { ReadInt8(data); return *this; }
    RawDataBuffer& operator>>(uint16_t& data) { ReadInt16(data); return *this; }
    RawDataBuffer& operator>>(uint32_t& data) { ReadInt32(data); return *this; }
    RawDataBuffer& operator>>(float& data) { ReadFloat(data); return *this; }
    RawDataBuffer& operator>>(std::string& data) { ReadStringUTF8(data); return *this; }



	ERROR_CODE ExtractData(std::vector<BYTE>& sendBuffer);
    ERROR_CODE ExtractData(BYTE* sendBuffer);

    size_t GetSize() const {return buffer.size();}
    size_t GetCurrOffset() const {return m_offset;}
    void SetOffset(size_t offset) {m_offset = offset;}
    bool IsValid() const {return m_isValid;}

    void Clear() 
    {
        m_isValid = true;
        m_offset = 0;
        buffer.clear();
    }

    void ShowData()
    {
        for (auto it = buffer.cbegin(); it != buffer.cend(); it++)
        {
            printf("%02X ", *it);
        }
    }
};


#endif