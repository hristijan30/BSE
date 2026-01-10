#include "Server.h"

#include <iostream>
#include <cstring>
#include <algorithm>

namespace BSE
{
    NetServer::NetServer()
        : host_(nullptr)
        , nextPeerId_(1)
    {
    }

    NetServer::~NetServer()
    {
        Stop();
    }

    bool NetServer::Start(uint16_t port)
    {
        if (!BSE::NET::Initialize())
        {
            std::cerr << "[NetServer] Failed to initialize ENet library\n";
            return false;
        }

        if (host_)
        {
            std::cerr << "[NetServer] Server already started\n";
            return false;
        }

        ENetAddress address = BSE::NET::CreateAddress("", port);
        host_ = enet_host_create(&address, static_cast<size_t>(Net::NET_MAX_PEERS), /*channels*/ 3, 0, 0);
        if (!host_)
        {
            std::cerr << "[NetServer] Failed to create ENet host\n";
            BSE::NET::Shutdown();
            return false;
        }

        std::cout << "[NetServer] Listening on " << BSE::NET::AddressToString(address) << "\n";
        return true;
    }

    void NetServer::Stop()
    {
        if (!host_) return;

        for (auto& kv : peers_)
        {
            ENetPeer* peer = kv.first;
            enet_peer_disconnect(peer, 0);
        }

        enet_host_flush(host_);

        enet_host_destroy(host_);
        host_ = nullptr;

        peers_.clear();
        nextPeerId_ = 1;

        BSE::NET::Shutdown();
        std::cout << "[NetServer] Stopped\n";
    }

    void NetServer::Update(uint32_t timeoutMs)
    {
        if (!host_) return;

        ENetEvent event;
        while (enet_host_service(host_, &event, static_cast<enet_uint32>(timeoutMs)) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    OnConnect(event);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    OnReceive(event);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    OnDisconnect(event);
                    break;

                default:
                    break;
            }
        }
    }

    size_t NetServer::GetPeerCount() const
    {
        return peers_.size();
    }

    bool NetServer::KickPeer(uint32_t peerId)
    {
        for (auto& kv : peers_)
        {
            PeerInfo& info = kv.second;
            if (info.id == peerId)
            {
                ENetPeer* peer = info.peer;
                if (peer)
                {
                    enet_peer_disconnect(peer, 0);
                    if (host_) enet_host_flush(host_);
                    std::cout << "[NetServer] Kick requested for peer id=" << peerId << "\n";
                    return true;
                }
            }
        }

        std::cerr << "[NetServer] KickPeer: peer id not found: " << peerId << "\n";
        return false;
    }

    uint32_t NetServer::GetPeerID(std::string peerName)
    {
        for (const auto& kv : peers_)
        {
            const PeerInfo& info = kv.second;
            if (!info.peerName.empty() && info.peerName == peerName)
            {
                return info.id;
            }
        }
        return 0;
    }

    std::string NetServer::GetPeerNameByID(uint32_t peerId) const
    {
        for (const auto& kv : peers_)
        {
            const PeerInfo& info = kv.second;
            if (info.id == peerId)
            {
                return info.peerName;
            }
        }
        return std::string{};
    }

    bool NetServer::SetNameForPeerByID(uint32_t peerId, const std::string& name)
    {
        for (auto& kv : peers_)
        {
            PeerInfo& info = kv.second;
            if (info.id == peerId)
            {
                info.peerName = name;
                std::cout << "[NetServer] Set name for peer id=" << peerId << " to \"" << name << "\"\n";
                return true;
            }
        }
        std::cerr << "[NetServer] SetNameForPeerByID: peer id not found: " << peerId << "\n";
        return false;
    }

    bool NetServer::SendToPeer(uint32_t peerId, const uint8_t* data, size_t len, bool reliable, uint8_t channel)
    {
        if (!host_) return false;
        if (!data || len == 0) return false;
        if (len > Net::NET_MAX_PACKET_SIZE)
        {
            std::cerr << "[NetServer] SendToPeer: packet too large: " << len << "\n";
            return false;
        }

        ENetPeer* target = nullptr;
        for (auto& kv : peers_)
        {
            PeerInfo& info = kv.second;
            if (info.id == peerId)
            {
                target = info.peer;
                break;
            }
        }

        if (!target)
        {
            std::cerr << "[NetServer] SendToPeer: peer id not found: " << peerId << "\n";
            return false;
        }

        enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        ENetPacket* packet = enet_packet_create(data, static_cast<size_t>(len), flags);
        if (!packet)
        {
            std::cerr << "[NetServer] SendToPeer: failed to create packet\n";
            return false;
        }

        if (enet_peer_send(target, channel, packet) != 0)
        {
            std::cerr << "[NetServer] SendToPeer: enet_peer_send failed\n";
            enet_packet_destroy(packet);
            return false;
        }

        enet_host_flush(host_);
        return true;
    }

    void NetServer::Broadcast(const uint8_t* data, size_t len, bool reliable, uint8_t channel, uint32_t excludePeerId)
    {
        if (!host_) return;
        if (!data || len == 0) return;
        if (len > Net::NET_MAX_PACKET_SIZE)
        {
            std::cerr << "[NetServer] Broadcast: packet too large: " << len << "\n";
            return;
        }

        enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;

        for (auto& kv : peers_)
        {
            PeerInfo& info = kv.second;
            if (excludePeerId != 0 && info.id == excludePeerId) continue;
            ENetPeer* peer = info.peer;
            if (!peer) continue;

            ENetPacket* pkt = enet_packet_create(data, static_cast<size_t>(len), flags);
            if (!pkt)
            {
                std::cerr << "[NetServer] Broadcast: failed to create packet for peer id=" << info.id << "\n";
                continue;
            }

            if (enet_peer_send(peer, channel, pkt) != 0)
            {
                std::cerr << "[NetServer] Broadcast: enet_peer_send failed for peer id=" << info.id << "\n";
                enet_packet_destroy(pkt);
                continue;
            }
        }

        enet_host_flush(host_);
    }

    void NetServer::OnConnect(ENetEvent& event)
    {
        ENetPeer* peer = event.peer;
        PeerInfo info;
        info.id = nextPeerId_++;
        info.peer = peer;
        info.lastHeard = std::chrono::steady_clock::now();
        peers_.emplace(peer, info);

        peer->data = reinterpret_cast<void*>(static_cast<uintptr_t>(info.id));

        std::cout << "[NetServer] Peer connected (id=" << info.id << " addr=" 
                << BSE::NET::AddressToString(peer->address) << ")\n";
    }

    void NetServer::OnReceive(ENetEvent& event)
    {
        ENetPeer* from = event.peer;
        const uint8_t* data = event.packet->data;
        size_t len = static_cast<size_t>(event.packet->dataLength);

        if (!ValidateIncomingPacket(data, len))
        {
            std::cerr << "[NetServer] Dropping invalid packet (len=" << len << ")\n";
            return;
        }

        DataSerializer reader(const_cast<uint8_t*>(data), len, false);
        uint16_t protocolVersion = 0;
        if (!reader.Read<uint16_t>(protocolVersion))
        {
            std::cerr << "[NetServer] Failed to read protocol version\n";
            return;
        }

        uint8_t rawType = 0;
        if (!reader.Read<uint8_t>(rawType))
        {
            std::cerr << "[NetServer] Failed to read packet type\n";
            return;
        }
        Net::PacketType type = static_cast<Net::PacketType>(rawType);

        if (protocolVersion != Net::NET_PROTOCOL_VERSION)
        {
            std::cerr << "[NetServer] Protocol version mismatch from peer (got=" << protocolVersion
                    << " expected=" << Net::NET_PROTOCOL_VERSION << ")\n";
            return;
        }

        auto pit = peers_.find(from);
        if (pit != peers_.end())
        {
            pit->second.lastHeard = std::chrono::steady_clock::now();
        }

        if (type == Net::PacketType::Handshake)
        {
            if (!HandleHandshake(from, reader))
            {
                std::cerr << "[NetServer] HandleHandshake failed for peer\n";
            }
            return;
        }

        enet_uint32 flags = 0;
        uint8_t channel = 1;
        GetSendFlagsForType(type, flags, channel);
        bool reliable = (flags & ENET_PACKET_FLAG_RELIABLE) != 0;

        RelayPacket(data, len, from, reliable, channel);
    }

    void NetServer::OnDisconnect(ENetEvent& event)
    {
        ENetPeer* peer = event.peer;
        auto it = peers_.find(peer);
        if (it != peers_.end())
        {
            uint32_t id = it->second.id;
            std::cout << "[NetServer] Peer disconnected (id=" << id << ")\n";
            peers_.erase(it);
        }
        else
        {
            std::cout << "[NetServer] Unknown peer disconnected\n";
        }
        peer->data = nullptr;
    }

    bool NetServer::ValidateIncomingPacket(const uint8_t* data, size_t len)
    {
        if (!data) return false;
        if (len == 0) return false;
        if (len > Net::NET_MAX_PACKET_SIZE)
        {
            std::cerr << "[NetServer] Packet too large: " << len << " bytes\n";
            return false;
        }

        if (len < (sizeof(uint16_t) + sizeof(uint8_t)))
        {
            std::cerr << "[NetServer] Packet too small for header\n";
            return false;
        }

        uint8_t rawType = data[sizeof(uint16_t)];
        if (rawType > static_cast<uint8_t>(Net::PacketType::Disconnect))
        {
            std::cerr << "[NetServer] Unknown packet type: " << static_cast<int>(rawType) << "\n";
            return false;
        }

        return true;
    }

    void NetServer::RelayPacket(const uint8_t* data, size_t len, ENetPeer* exclude, bool reliable, uint8_t channel)
    {
        if (!host_) return;
        if (peers_.empty()) return;

        enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        ENetPacket* outPacket = enet_packet_create(data, static_cast<size_t>(len), flags);
        if (!outPacket)
        {
            std::cerr << "[NetServer] Failed to allocate relay packet\n";
            return;
        }

        for (auto& kv : peers_)
        {
            ENetPeer* peer = kv.first;
            if (peer == exclude) continue;

            ENetPacket* packetCopy = enet_packet_create(data, static_cast<size_t>(len), flags);
            if (!packetCopy)
            {
                std::cerr << "[NetServer] Failed to create packet copy for peer\n";
                continue;
            }

            if (enet_peer_send(peer, channel, packetCopy) != 0)
            {
                std::cerr << "[NetServer] enet_peer_send failed for a peer\n";
                enet_packet_destroy(packetCopy);
            }
        }

        enet_host_flush(host_);

        enet_packet_destroy(outPacket);
    }
    
    bool NetServer::HandleHandshake(ENetPeer* fromPeer, DataSerializer& reader)
    {
        std::string playerName;
        if (!reader.ReadString(playerName))
        {
            std::cerr << "[NetServer] HandleHandshake: failed to read player name\n";
            return false;
        }

        auto it = peers_.find(fromPeer);
        if (it == peers_.end())
        {
            std::cerr << "[NetServer] HandleHandshake: unknown peer\n";
            return false;
        }

        PeerInfo& info = it->second;
        info.peerName = playerName;

        std::cout << "[NetServer] Peer id=" << info.id << " set name='" << playerName << "'\n";

        {
            DataSerializer writer(Net::NET_MAX_PACKET_SIZE);
            writer.Write<uint16_t>(Net::NET_PROTOCOL_VERSION);
            writer.Write<uint8_t>(static_cast<uint8_t>(Net::PacketType::Handshake));
            writer.Write<uint32_t>(info.id);
            writer.WriteString(std::string("Welcome"));

            SendToPeer(info.id, writer.GetBuffer(), writer.GetSizeWritten(), /*reliable=*/true, /*channel=*/0);
        }

        {
            DataSerializer writer(Net::NET_MAX_PACKET_SIZE);
            writer.Write<uint16_t>(Net::NET_PROTOCOL_VERSION);
            writer.Write<uint8_t>(static_cast<uint8_t>(Net::PacketType::Event));
            writer.Write<uint16_t>(1);
            writer.Write<uint32_t>(info.id);
            writer.WriteString(info.peerName);

            Broadcast(writer.GetBuffer(), writer.GetSizeWritten(), /*reliable=*/true, /*channel=*/0, /*excludePeerId=*/info.id);
        }

        return true;
    }

    void NetServer::GetSendFlagsForType(Net::PacketType type, enet_uint32& outFlags, uint8_t& outChannel) const
    {
        outFlags = 0;
        outChannel = 1;

        switch (type)
        {
            case Net::PacketType::Handshake:
            case Net::PacketType::Event:
            case Net::PacketType::Disconnect:
                outFlags = ENET_PACKET_FLAG_RELIABLE;
                outChannel = 0;
                break;

            case Net::PacketType::Input:
            case Net::PacketType::State:
                outFlags = 0;
                outChannel = 1;
                break;

            case Net::PacketType::Ping:
                outFlags = ENET_PACKET_FLAG_RELIABLE;
                outChannel = 0;
                break;

            default:
                outFlags = 0;
                outChannel = 1;
                break;
        }
    }
}
