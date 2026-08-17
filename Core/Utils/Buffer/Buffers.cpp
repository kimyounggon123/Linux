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
        int size = GetCurrDataSize(); // 현재 데이터 양 학인
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
ERROR_CODE RawDataBuffer::AppendRawMemory(const BYTE* src, const size_t& size)
{
	buffer.insert(buffer.end(), src, src + size);
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE RawDataBuffer::PushBytes(const BYTE* src, const size_t& size)
{
	if (buffer.size() + size > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
	if (src == nullptr) return ERROR_CODE::GET_NULLPTR;
	return AppendRawMemory(src, size);
}

ERROR_CODE RawDataBuffer::OverlapBytes(const BYTE* src, const size_t& size, size_t& offset)
{
	if (size == 0) return ERROR_CODE::SUCCESS;
	if (src == nullptr) return ERROR_CODE::GET_NULLPTR;
	memcpy(buffer.data() + offset, src, size);
	return ERROR_CODE::SUCCESS;
}	



ERROR_CODE RawDataBuffer::PushInt8(const uint8_t& src)
{
	if (buffer.size() + sizeof(uint8_t) > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
	return AppendRawMemory(&src, sizeof(uint8_t));
}
ERROR_CODE RawDataBuffer::PushInt16(const uint16_t& src)
{
	if (buffer.size() + sizeof(uint16_t) > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
	uint16_t final_value = SwapIfNeeded(src);
    return AppendRawMemory(reinterpret_cast<const BYTE*>(&final_value), sizeof(uint16_t));
}
ERROR_CODE RawDataBuffer::PushInt32(const uint32_t& src)
{
	if (buffer.size() + sizeof(uint32_t) > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
    uint32_t final_value = SwapIfNeeded(src);
    return AppendRawMemory(reinterpret_cast<const BYTE*>(&final_value), sizeof(uint32_t));
}
ERROR_CODE RawDataBuffer::PushFloat(const float& src)
{
	if (buffer.size() + sizeof(uint32_t) > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
	uint32_t final_value;
	std::memcpy(&final_value, &src, 4);
	return PushInt32(final_value);
}

// string input methods
ERROR_CODE RawDataBuffer::PushStringUTF8(const std::string& src)
{
	uint32_t str_len = static_cast<uint32_t>(src.size());
	ERROR_CODE code = PushInt32(str_len);
	if (code != ERROR_CODE::SUCCESS) return code;

	if (buffer.size() + str_len > buffer.capacity()) return ERROR_CODE::MEMORY_LIMIT;
	code = PushBytes(reinterpret_cast<const BYTE*>(src.data()), str_len); // copy string data
	return ERROR_CODE::SUCCESS;
}

/// Read control methods
ERROR_CODE RawDataBuffer::ReadRawMemory(BYTE* dest, const size_t& size)
{
	memcpy(dest, &buffer[m_offset], size);
	m_offset += size;
	return ERROR_CODE::SUCCESS;
}


ERROR_CODE RawDataBuffer::ReadBytes(BYTE* dest, const size_t& size)
{
	if (dest == nullptr) return ERROR_CODE::GET_NULLPTR;
	if (size == 0) return ERROR_CODE::SUCCESS;
	if (!CanReadThisPart(size)) return ERROR_CODE::INCORRECT_SIZE;

	ReadRawMemory(dest, size);
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE RawDataBuffer::ReadInt8(uint8_t& dest)
{
	if (!CanReadThisPart(sizeof(uint8_t))) return ERROR_CODE::INCORRECT_SIZE;
	return ReadRawMemory(&dest, sizeof(uint8_t));
}
ERROR_CODE RawDataBuffer::ReadInt16(uint16_t& dest)
{
	if (!CanReadThisPart(sizeof(uint16_t))) return ERROR_CODE::INCORRECT_SIZE;
	
	uint16_t rawdata = 0;
	ReadRawMemory(reinterpret_cast<BYTE*>(&rawdata), sizeof(uint16_t));
	dest = SwapIfNeeded(rawdata);
	return ERROR_CODE::SUCCESS;
}
ERROR_CODE RawDataBuffer::ReadInt32(uint32_t& dest)
{
	if (!CanReadThisPart(sizeof(uint32_t))) return ERROR_CODE::INCORRECT_SIZE;
	
	uint32_t rawdata = 0;
	ReadRawMemory(reinterpret_cast<BYTE*>(&rawdata), sizeof(uint32_t));
	dest = SwapIfNeeded(rawdata);
	return ERROR_CODE::SUCCESS;
}
ERROR_CODE RawDataBuffer::ReadFloat(float& dest)
{
	if (!CanReadThisPart(sizeof(uint32_t))) return ERROR_CODE::INCORRECT_SIZE;
	uint32_t final_bits = 0;
    ERROR_CODE result = ReadInt32(final_bits);
    if (result != ERROR_CODE::SUCCESS) return result;
	std::memcpy(&dest, &final_bits, sizeof(float));
    return ERROR_CODE::SUCCESS;
}


// string input methods
ERROR_CODE RawDataBuffer::ReadStringUTF8(std::string& dest)
{
	uint32_t str_len = 0;
	ERROR_CODE code = ReadInt32(str_len);
	if (code != ERROR_CODE::SUCCESS) return code;

	if (m_offset + str_len > buffer.size()) return ERROR_CODE::INCORRECT_SIZE;

	dest.resize(str_len);
	
	return ReadBytes(reinterpret_cast<BYTE*>(&dest[0]), str_len);
}


ERROR_CODE RawDataBuffer::ExtractData(std::vector<BYTE>& sendBuffer)
{
	sendBuffer.insert(sendBuffer.end(), buffer.begin(), buffer.end());
	return ERROR_CODE::SUCCESS;
}

ERROR_CODE RawDataBuffer::ExtractData(BYTE* sendBuffer)
{
	if (sendBuffer == nullptr) return ERROR_CODE::GET_NULLPTR;
	if (buffer.size() == 0) return ERROR_CODE::EMPTY_CONTAINOR;

	memcpy(sendBuffer, buffer.data(), buffer.size());
		
	return ERROR_CODE::SUCCESS;
}



