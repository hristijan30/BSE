#pragma once

#include "../Engine/StandardInclude.h"

namespace BSE::Net
{
    constexpr uint16_t NET_PROTOCOL_VERSION = 1;
    constexpr size_t   NET_MAX_PACKET_SIZE  = 5000;

    constexpr uint32_t NET_DEFAULT_PORT = 27015;
    extern uint32_t NET_MAX_PEERS;
}
