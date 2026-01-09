#include "DataSerializer.h"
#include <cstring>

namespace BSE
{
    DataSerializer::DataSerializer(uint8_t* buffer, size_t capacity, bool takeOwnership)
        : buffer_(buffer)
        , capacity_(capacity)
        , offset_(0)
        , owns_(takeOwnership)
    {}

    DataSerializer::DataSerializer(size_t capacity)
        : buffer_(nullptr)
        , capacity_(capacity)
        , offset_(0)
        , owns_(true)
    {
        buffer_ = new uint8_t[capacity_];
        memset(buffer_, 0, capacity_);
    }

    DataSerializer::~DataSerializer()
    {
        if (owns_ && buffer_)
        {
            delete [] buffer_;
            buffer_ = nullptr;
        }
    }

    void DataSerializer::Reset()
    {
        offset_ = 0;
    }

    size_t DataSerializer::GetOffset() const { return offset_; }

    void DataSerializer::SetOffset(size_t offset)
    {
        if (offset <= capacity_) offset_ = offset;
        else offset_ = capacity_;
    }

    uint8_t* DataSerializer::GetBuffer() { return buffer_; }
    const uint8_t* DataSerializer::GetBuffer() const { return buffer_; }
    size_t DataSerializer::GetCapacity() const { return capacity_; }
    size_t DataSerializer::GetSizeWritten() const { return offset_; }

    bool DataSerializer::WriteBytes(const void* src, size_t len)
    {
        if (!src) return false;
        if (len == 0) return true;
        if (offset_ + len > capacity_) return false;
        memcpy(buffer_ + offset_, src, len);
        offset_ += len;
        return true;
    }

    bool DataSerializer::ReadBytes(void* dst, size_t len)
    {
        if (!dst) return false;
        if (len == 0) return true;
        if (offset_ + len > capacity_) return false;
        memcpy(dst, buffer_ + offset_, len);
        offset_ += len;
        return true;
    }
}
