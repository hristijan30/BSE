#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#if __has_include(<bit>)
  #include <bit>
#endif

namespace BSE
{
    class DLL_EXPORT DataSerializer
    {
    public:
        DataSerializer(uint8_t* buffer, size_t capacity, bool takeOwnership = false);
        explicit DataSerializer(size_t capacity);
        ~DataSerializer();

        void Reset();

        size_t GetOffset() const;
        void   SetOffset(size_t offset);

        uint8_t* GetBuffer();
        const uint8_t* GetBuffer() const;
        size_t GetCapacity() const;
        size_t GetSizeWritten() const;

        template<typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, bool>
        Write(const T& value)
        {
            return WriteArithmetic(value);
        }

        template<typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, bool>
        Read(T& out)
        {
            return ReadArithmetic(out);
        }

        bool WriteBytes(const void* src, size_t len);
        bool ReadBytes(void* dst, size_t len);

        bool WriteString(const std::string& s)
        {
            if (s.size() > 0xFFFFu) return false;
            uint16_t len = static_cast<uint16_t>(s.size());
            if (!Write<uint16_t>(len)) return false;
            return WriteBytes(s.data(), len);
        }

        bool ReadString(std::string& out)
        {
            uint16_t len = 0;
            if (!Read<uint16_t>(len)) return false;
            if (len == 0) { out.clear(); return true; }
            out.resize(len);
            return ReadBytes(out.data(), len);
        }

    private:
        template<typename T>
        bool WriteArithmetic(const T& value)
        {
            if constexpr (std::is_integral_v<T>)
            {
                using UT = std::make_unsigned_t<T>;
                UT v = static_cast<UT>(value);
                size_t n = sizeof(UT);
                if (offset_ + n > capacity_) return false;
                for (size_t i = 0; i < n; ++i)
                {
                    buffer_[offset_ + i] = static_cast<uint8_t>(v & 0xFFu);
                    v >>= 8;
                }
                offset_ += n;
                return true;
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (sizeof(T) == sizeof(uint32_t))
                {
                    uint32_t tmp = portable_bit_cast<uint32_t, T>(value);
                    return WriteArithmetic<uint32_t>(tmp);
                }
                else if constexpr (sizeof(T) == sizeof(uint64_t))
                {
                    uint64_t tmp = portable_bit_cast<uint64_t, T>(value);
                    return WriteArithmetic<uint64_t>(tmp);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        template<typename T>
        bool ReadArithmetic(T& out)
        {
            if constexpr (std::is_integral_v<T>)
            {
                using UT = std::make_unsigned_t<T>;
                size_t n = sizeof(UT);
                if (offset_ + n > capacity_) return false;
                UT v = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    v |= (static_cast<UT>(buffer_[offset_ + i]) << (8 * i));
                }
                offset_ += n;
                out = static_cast<T>(v);
                return true;
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (sizeof(T) == sizeof(uint32_t))
                {
                    uint32_t tmp = 0;
                    if (!ReadArithmetic<uint32_t>(tmp)) return false;
                    out = portable_bit_cast<T, uint32_t>(tmp);
                    return true;
                }
                else if constexpr (sizeof(T) == sizeof(uint64_t))
                {
                    uint64_t tmp = 0;
                    if (!ReadArithmetic<uint64_t>(tmp)) return false;
                    out = portable_bit_cast<T, uint64_t>(tmp);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        uint8_t* buffer_ = nullptr;
        size_t   capacity_ = 0;
        size_t   offset_ = 0;
        bool     owns_ = false;
    };
}
