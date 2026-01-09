#pragma once

#include "ENet.h"
#include "NetConfig.h"
#include "PacketTypes.h"
#include "DataSerializer.h"

namespace BSE
{
    class DLL_EXPORT NetClient
    {
    public:
        NetClient();
        ~NetClient();

        bool Connect(const std::string& ip, uint16_t port, uint32_t timeoutMs = 5000);
        void Disconnect();

        void Update(uint32_t timeoutMs = 0);

        bool SendPacket(Net::PacketType type, const void* payload, size_t payloadLen);

        bool IsConnected() const;

        std::function<void()> onConnected;
        std::function<void()> onDisconnected;
        std::function<void(const uint8_t*, size_t, Net::PacketType)> onPacket;

    private:
        ENetHost* host_ = nullptr;
        ENetPeer* serverPeer_ = nullptr;
        bool connected_ = false;
        std::chrono::steady_clock::time_point lastHeard_;

        bool ValidateIncomingPacket(const uint8_t* data, size_t len) const;
        void HandleConnect(ENetEvent& event);
        void HandleReceive(ENetEvent& event);
        void HandleDisconnect(ENetEvent& event);

        void GetSendFlagsForType(Net::PacketType type, enet_uint32& outFlags, uint8_t& outChannel) const;
    };
}
