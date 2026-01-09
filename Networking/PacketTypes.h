#pragma once

#include <cstdint>

namespace BSE::Net
{
    enum class PacketType : uint8_t {
        Handshake,
        Input,
        State,
        Event,
        Ping,
        Disconnect
    };

    struct PacketHeader
    {
        uint16_t protocolVersion;
        PacketType type;
    };
}
