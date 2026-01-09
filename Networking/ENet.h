#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include <enet/enet.h>

namespace BSE::NET
{
    inline bool Initialize()
    {
        if (enet_initialize() != 0)
        {
            std::cerr << "[FATAL]: Failed Initialization" << std::endl;
            return false;
        }

        return true;
    }

    inline void Shutdown()
    {
        enet_deinitialize();
    }

    inline ENetAddress CreateAddress(const std::string& ip, uint16_t port)
    {
        ENetAddress address{};
        address.port = port;

        if (ip.empty() || ip == "0.0.0.0")
        {
            address.host = ENET_HOST_ANY;
        }
        else
        {
            if (enet_address_set_host(&address, ip.c_str()) != 0)
            {
                address.host = ENET_HOST_ANY;
            }
        }

        return address;
    }

    inline std::string AddressToString(const ENetAddress& address)
    {
        char ip[64] = {};
        enet_address_get_host_ip(&address, ip, sizeof(ip));

        return std::string(ip) + ":" + std::to_string(address.port);
    }
}
