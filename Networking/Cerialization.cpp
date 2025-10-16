#include "Cerialization.h"
#include <cstring>

namespace BSE
{
    Cerialization& Cerialization::Instance()
    {
        static Cerialization inst;
        return inst;
    }

    Cerialization::ByteBuffer Cerialization::SerializeRaw(uint32_t typeId, const std::any& obj) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_serializers.find(typeId);
        if (it == m_serializers.end())
            throw std::runtime_error("Cerialization::SerializeRaw: typeId not registered");
        return it->second(obj);
    }

    std::any Cerialization::DeserializeRaw(uint32_t typeId, const ByteBuffer& data) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_deserializers.find(typeId);
        if (it == m_deserializers.end())
            throw std::runtime_error("Cerialization::DeserializeRaw: typeId not registered");
        return it->second(data);
    }

    bool Cerialization::HasType(uint32_t typeId) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_serializers.count(typeId) != 0 && m_deserializers.count(typeId) != 0;
    }
}
