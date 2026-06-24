#include "Buffers.hpp"

void RecvBuffer::OnWrite(int size) { write_pos += size; }
void RecvBuffer::OnRead(int size)
{
    read_pos += size;
    if (read_pos >= write_pos)
    { // 만약 다 읽었다면 다시 초기화
        Clear();
    }
}

void RecvBuffer::Clear()
{
    read_pos = 0;
    write_pos = 0;   
}

// 남은 버퍼 내 정보를 앞으로 당기는 method
void RecvBuffer::MoveDataFront()
{
    // 뒤쪽에 데이터가 있다면
    if (read_pos > 0)
    {
        int size = GetRemainSize(); // 현재 데이터 양 학인
        if (size > 0)
        {
            // 데이터 있으면 데이터를 가장 앞으로 이동
            memmove(buffer, &buffer[read_pos], size);
        }
        read_pos = 0; // read index 초기화
        write_pos = size; // write index는 데이터 크기만큼 이동.
    }
}



/// Input control methods
void RawDataBuffer::AppendRawMemory(const BYTE* src, const size_t& size)
{
	buffer.insert(buffer.end(), src, src + size);
}

ERROR_CODE RawDataBuffer::PushBytes(const BYTE* src, const size_t& size)
{
	if (src == nullptr) return ERROR_CODE::GET_NULLPTR;
	AppendRawMemory(src, size);
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE RawDataBuffer::OverlapBytes(const BYTE* src, const size_t& size, size_t& offset)
{
	if (size == 0) return ERROR_CODE::SUCCESS;
	if (src == nullptr) return ERROR_CODE::GET_NULLPTR;
	memcpy(buffer.data() + offset, src, size);
	return ERROR_CODE::SUCCESS;
}	

template <typename T>
ERROR_CODE RawDataBuffer::PushNumericData(const T& src)
{
	size_t dataSize = sizeof(T);
	T final_value = src;
	
	if constexpr (sizeof(T) == 1)
	{
		//final_value = final_value;
	}

	else if constexpr (sizeof(T) == 2)
	{
		uint16_t raw_bits = reinterpret_cast<const uint16_t&>(src);
		uint16_t swap_bits = bigEndianFlag ? htons(raw_bits) : raw_bits;
		final_value = reinterpret_cast<T&>(swap_bits);
		// bigEndianFlag ? static_cast<T>(htons(static_cast<uint16_t>(src)))
		// 			: static_cast<T>(static_cast<uint16_t>(src));
	}

	else if constexpr (sizeof(T) == 4)
	{
		uint32_t raw_bits = reinterpret_cast<const uint32_t&>(src);
		uint32_t swap_bits = bigEndianFlag ? htonl(raw_bits) : raw_bits;
		final_value = reinterpret_cast<T&>(swap_bits);
	}

	else return ERROR_CODE::FUNCTION_RUNNING_FAILED;

	// 어떤 타입이든 주소를 얻어서 BYTE*로 캐스팅
	const BYTE* srcBytes = reinterpret_cast<const BYTE*>(&final_value);
	AppendRawMemory(srcBytes, sizeof(T));
	return ERROR_CODE::SUCCESS;
}


// num input methods
ERROR_CODE RawDataBuffer::PushInt(const int32_t& src)
{
	return PushNumericData(src);
}
ERROR_CODE RawDataBuffer::PushFloat(const float& src)
{
	return PushNumericData(src);
}

// string input methods
ERROR_CODE RawDataBuffer::PushStringUTF8(const std::string& src)
{
	int str_len = static_cast<int>(src.size());

	//ERROR_CODE code = InputNumericData(static_cast<BYTE>(1), offset); 
	//if (code != ERROR_CODE::SUCCESS) return code;

	ERROR_CODE code = PushNumericData(str_len);
	if (code != ERROR_CODE::SUCCESS) return code;
	code = PushBytes(reinterpret_cast<const BYTE*>(src.data()), str_len); // copy string data

	return ERROR_CODE::SUCCESS;
}

/// Read control methods
void RawDataBuffer::ReadRawMemory(BYTE* dest, const size_t& size, size_t& offset)
{
	memcpy(dest, &buffer[offset], size);
	offset += size;
}


ERROR_CODE RawDataBuffer::ReadBytes(BYTE* dest, const size_t& size, size_t& offset)
{
	if (dest == nullptr) return ERROR_CODE::GET_NULLPTR;
	if (size == 0) return ERROR_CODE::SUCCESS;
	if (offset + size > buffer.size()) return ERROR_CODE::INCORRECT_SIZE;

	ReadRawMemory(dest, size, offset);
	return ERROR_CODE::SUCCESS;
}

template <typename T>
ERROR_CODE RawDataBuffer::ReadNumericData(T& dest, size_t& offset)
{
	if (offset + sizeof(T) > buffer.size()) return ERROR_CODE::INCORRECT_SIZE;
	T raw_value;
	ReadRawMemory(reinterpret_cast<BYTE*>(&raw_value), sizeof(T), offset);

	if constexpr (sizeof(T) == 1)
	{
		dest = raw_value;
	}
	else if constexpr (sizeof(T) == 2)
	{
		uint16_t net_bits;
		memcpy(&net_bits, &raw_value, 2);

		uint16_t host_bits = bigEndianFlag ? ntohs(net_bits) : net_bits;
		memcpy(&dest, &host_bits, 2);
	}
	else if constexpr (sizeof(T) == 4)
	{
		uint32_t net_bits;
		memcpy(&net_bits, &raw_value, 4);

		uint32_t host_bits = bigEndianFlag ? ntohl(net_bits) : net_bits;
		memcpy(&dest, &host_bits, 4);
	}
	else
	{
		return ERROR_CODE::FUNCTION_RUNNING_FAILED;
	}

	return ERROR_CODE::SUCCESS;
}


ERROR_CODE RawDataBuffer::ReadInt(int32_t& dest, size_t& offset)
{
	return ReadNumericData(dest, offset);
}
ERROR_CODE RawDataBuffer::ReadFloat(float& dest, size_t& offset)
{
	return ReadNumericData(dest, offset);
}
ERROR_CODE RawDataBuffer::ReadDouble(double& dest, size_t& offset)
{
	return ReadNumericData(dest, offset);
}

// string input methods
ERROR_CODE RawDataBuffer::ReadStringUTF8(std::string& dest, size_t& offset)
{
	uint32_t str_len = 0;
	ERROR_CODE code = ReadNumericData(str_len, offset);
	if (code != ERROR_CODE::SUCCESS) return code;

	if (offset + str_len > buffer.size()) return ERROR_CODE::INCORRECT_SIZE;

	dest.resize(str_len);
	
	return ReadBytes(reinterpret_cast<BYTE*>(&dest[0]), str_len, offset);
}


ERROR_CODE RawDataBuffer::ExtractData(std::vector<BYTE>& sendBuffer)
{
	if (buffer.size() == 0) return ERROR_CODE::EMPTY_CONTAINOR;
	sendBuffer.insert(sendBuffer.end(), buffer.begin(), buffer.end());
	return ERROR_CODE::SUCCESS;
}
ERROR_CODE RawDataBuffer::ExtractData(BYTE* sendBuffer, size_t& offset)
{
	if (sendBuffer == nullptr) return ERROR_CODE::GET_NULLPTR;
	if (buffer.size() == 0) return ERROR_CODE::EMPTY_CONTAINOR;
	
	memcpy(sendBuffer + offset, buffer.data(), buffer.size());
	offset += buffer.size();
		
	return ERROR_CODE::SUCCESS;
}



