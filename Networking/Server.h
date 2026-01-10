#pragma once

#include "ENet.h"
#include "NetConfig.h"
#include "PacketTypes.h"
#include "DataSerializer.h"

namespace BSE
{
    class DLL_EXPORT NetServer
    {
    public:
        NetServer();
        ~NetServer();

        bool Start(uint16_t port = Net::NET_DEFAULT_PORT);
        void Stop();

        void Update(uint32_t timeoutMs = 0);
        size_t GetPeerCount() const;

        bool KickPeer(uint32_t peerId);
        uint32_t GetPeerID(std::string peerName);
        std::string GetPeerNameByID(uint32_t peerId) const;
        bool SetNameForPeerByID(uint32_t peerId, const std::string& name);

        bool SendToPeer(uint32_t peerId, const uint8_t* data, size_t len, bool reliable = false, uint8_t channel = 1);
        void Broadcast(const uint8_t* data, size_t len, bool reliable = false, uint8_t channel = 1, uint32_t excludePeerId = 0);

    private:
        struct PeerInfo
        {
            uint32_t id = 0;
            std::string peerName;
            ENetPeer* peer = nullptr;
            std::chrono::steady_clock::time_point lastHeard;
        };

        ENetHost* host_ = nullptr;
        uint32_t nextPeerId_ = 1;
        std::unordered_map<ENetPeer*, PeerInfo> peers_;

        void OnConnect(ENetEvent& event);
        void OnReceive(ENetEvent& event);
        void OnDisconnect(ENetEvent& event);

        bool ValidateIncomingPacket(const uint8_t* data, size_t len);

        void RelayPacket(const uint8_t* data, size_t len, ENetPeer* exclude, bool reliable, uint8_t channel = 1);

        void GetSendFlagsForType(Net::PacketType type, enet_uint32& outFlags, uint8_t& outChannel) const;
    };
}
