#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include <any>
#include <cstring>

namespace BSE
{
    class DLL_EXPORT Cerialization
    {
    public:
        using ByteBuffer = std::vector<uint8_t>;
        using SerializerFunc   = std::function<ByteBuffer(const std::any&)>;
        using DeserializerFunc = std::function<std::any(const ByteBuffer&)>;

        static Cerialization& Instance();

        template<typename T>
        void RegisterType(uint32_t typeId,
                          std::function<ByteBuffer(const T&)> serializer,
                          std::function<T(const ByteBuffer&)> deserializer)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_serializers.count(typeId) || m_deserializers.count(typeId))
                throw std::runtime_error("Cerialization::RegisterType: typeId already registered");

            m_serializers[typeId] = [serializer](const std::any& anyObj) -> ByteBuffer {
                return serializer(std::any_cast<const T&>(anyObj));
            };
            m_deserializers[typeId] = [deserializer](const ByteBuffer& buf) -> std::any {
                return std::any(deserializer(buf));
            };
        }

        ByteBuffer SerializeRaw(uint32_t typeId, const std::any& obj) const;
        std::any  DeserializeRaw(uint32_t typeId, const ByteBuffer& data) const;

        template<typename T>
        ByteBuffer Serialize(uint32_t typeId, const T& obj) const
        {
            return SerializeRaw(typeId, std::any(obj));
        }

        template<typename T>
        T Deserialize(uint32_t typeId, const ByteBuffer& data) const
        {
            std::any anyObj = DeserializeRaw(typeId, data);
            return std::any_cast<T>(anyObj);
        }

        bool HasType(uint32_t typeId) const;

    private:
        Cerialization() = default;
        ~Cerialization() = default;
        Cerialization(const Cerialization&) = delete;
        Cerialization& operator=(const Cerialization&) = delete;

        std::unordered_map<uint32_t, SerializerFunc>   m_serializers;
        std::unordered_map<uint32_t, DeserializerFunc> m_deserializers;
        mutable std::mutex m_mutex;
    };

    template<typename T>
    inline void AppendPod(std::vector<uint8_t>& dst, const T& value)
    {
        static_assert(std::is_trivially_copyable<T>::value, "AppendPod requires trivially copyable type");
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
        dst.insert(dst.end(), p, p + sizeof(T));
    }

    template<typename T>
    inline T ReadPod(const std::vector<uint8_t>& src, size_t offset)
    {
        static_assert(std::is_trivially_copyable<T>::value, "ReadPod requires trivially copyable type");
        if (offset + sizeof(T) > src.size())
            throw std::out_of_range("ReadPod: out of range");
        T v;
        std::memcpy(&v, src.data() + offset, sizeof(T));
        return v;
    }

    inline void AppendString(std::vector<uint8_t>& dst, const std::string& s)
    {
        uint32_t len = static_cast<uint32_t>(s.size());
        AppendPod(dst, len);
        dst.insert(dst.end(), reinterpret_cast<const uint8_t*>(s.data()), reinterpret_cast<const uint8_t*>(s.data()) + s.size());
    }

    inline std::string ReadString(const std::vector<uint8_t>& src, size_t& offset)
    {
        uint32_t len = ReadPod<uint32_t>(src, offset);
        offset += sizeof(uint32_t);
        if (offset + len > src.size()) throw std::out_of_range("ReadString: out of range");
        std::string s(reinterpret_cast<const char*>(src.data() + offset), len);
        offset += len;
        return s;
    }

    #define BSE_REGISTER_SERIALIZABLE(TYPE, ID, SER_FN, DESER_FN) \
        do { BSE::Cerialization::Instance().RegisterType<TYPE>(ID, SER_FN, DESER_FN); } while(0)
}