#include "Client.h"

namespace BSE
{
    NetClient::NetClient()
        : host_(nullptr)
        , serverPeer_(nullptr)
        , connected_(false)
    {}

    NetClient::~NetClient()
    {
        Disconnect();

        if (host_)
        {
            enet_host_destroy(host_);
            host_ = nullptr;
        }
    }

    bool NetClient::Connect(const std::string& ip, uint16_t port, uint32_t timeoutMs)
    {
        if (!BSE::NET::Initialize())
        {
            std::cerr << "[NetClient] ENet initialize failed\n";
            return false;
        }

        if (host_)
        {
            std::cerr << "[NetClient] Client already created\n";
            return false;
        }

        host_ = enet_host_create(nullptr, 1, /*channels*/ 3, 0, 0);
        if (!host_)
        {
            std::cerr << "[NetClient] Failed to create ENet client host\n";
            return false;
        }

        ENetAddress address = BSE::NET::CreateAddress(ip, port);
        serverPeer_ = enet_host_connect(host_, &address, /*channels*/ 3, 0);
        if (!serverPeer_)
        {
            std::cerr << "[NetClient] Failed to initiate connection\n";
            return false;
        }

        ENetEvent event;
        if (enet_host_service(host_, &event, timeoutMs) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            HandleConnect(event);
            return true;
        }

        return true;
    }

    void NetClient::Disconnect()
    {
        if (!host_ || !serverPeer_) return;

        if (connected_)
        {
            enet_peer_disconnect(serverPeer_, 0);
            ENetEvent event;
            while (enet_host_service(host_, &event, 100) > 0)
            {
                if (event.type == ENET_EVENT_TYPE_RECEIVE)
                {
                    enet_packet_destroy(event.packet);
                }
                else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
                {
                    break;
                }
            }
        }

        if (serverPeer_)
        {
            serverPeer_->data = nullptr;
            serverPeer_ = nullptr;
        }

        connected_ = false;

        if (onDisconnected) onDisconnected();

        if (host_)
        {
            enet_host_destroy(host_);
            host_ = nullptr;
        }
    }

    void NetClient::Update(uint32_t timeoutMs)
    {
        if (!host_) return;

        ENetEvent event;
        while (enet_host_service(host_, &event, static_cast<enet_uint32>(timeoutMs)) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    HandleConnect(event);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    HandleReceive(event);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    HandleDisconnect(event);
                    break;

                default:
                    break;
            }
        }
    }

    bool NetClient::SendHandshake(const std::string& playerName)
    {
        DataSerializer writer(Net::NET_MAX_PACKET_SIZE);

        writer.Write<uint16_t>(Net::NET_PROTOCOL_VERSION);
        writer.Write<uint8_t>(static_cast<uint8_t>(Net::PacketType::Handshake));
        writer.WriteString(playerName);

        ENetPacket* packet = enet_packet_create(
            writer.GetBuffer(),
            writer.GetSizeWritten(),
            ENET_PACKET_FLAG_RELIABLE
        );

        return enet_peer_send(serverPeer_, 0, packet) == 0;
    }

    bool NetClient::SendPacket(Net::PacketType type, const void* payload, size_t payloadLen)
    {
        if (!host_ || !serverPeer_ || !connected_) return false;

        const size_t headerSize = sizeof(uint16_t) + sizeof(uint8_t);
        const size_t total = headerSize + payloadLen;
        if (total > Net::NET_MAX_PACKET_SIZE)
        {
            std::cerr << "[NetClient] Attempt to send packet exceeding NET_MAX_PACKET_SIZE\n";
            return false;
        }

        std::unique_ptr<uint8_t[]> heapBuf;
        uint8_t stackBuf[2048];
        uint8_t* buf = nullptr;
        if (total <= sizeof(stackBuf))
        {
            buf = stackBuf;
        }
        else
        {
            heapBuf.reset(new uint8_t[total]);
            buf = heapBuf.get();
        }

        DataSerializer writer(buf, total, false);
        if (!writer.Write<uint16_t>(Net::NET_PROTOCOL_VERSION)) return false;
        if (!writer.Write<uint8_t>(static_cast<uint8_t>(type))) return false;
        if (payload && payloadLen > 0)
        {
            if (!writer.WriteBytes(payload, payloadLen)) return false;
        }

        enet_uint32 flags = 0;
        uint8_t channel = 1;
        GetSendFlagsForType(type, flags, channel);

        ENetPacket* packet = enet_packet_create(buf, total, flags);
        if (!packet)
        {
            std::cerr << "[NetClient] Failed to create ENet packet\n";
            return false;
        }

        if (enet_peer_send(serverPeer_, channel, packet) != 0)
        {
            std::cerr << "[NetClient] enet_peer_send failed\n";
            enet_packet_destroy(packet);
            return false;
        }

        enet_host_flush(host_);
        return true;
    }

    bool NetClient::IsConnected() const
    {
        return connected_;
    }

    void NetClient::HandleConnect(ENetEvent& event)
    {
        connected_ = true;
        lastHeard_ = std::chrono::steady_clock::now();

        if (event.peer)
        {
            event.peer->data = reinterpret_cast<void*>(1);
            serverPeer_ = event.peer;
        }

        std::cout << "[NetClient] Connected to server: " << (serverPeer_ ? BSE::NET::AddressToString(serverPeer_->address) : std::string("unknown")) << "\n";

        if (onConnected) onConnected();
    }

    void NetClient::HandleReceive(ENetEvent& event)
    {
        const uint8_t* data = event.packet->data;
        size_t len = static_cast<size_t>(event.packet->dataLength);

        if (!ValidateIncomingPacket(data, len))
        {
            std::cerr << "[NetClient] Dropping invalid packet (len=" << len << ")\n";
            return;
        }

        DataSerializer reader(const_cast<uint8_t*>(data), len, false);
        uint16_t protocolVersion = 0;
        if (!reader.Read<uint16_t>(protocolVersion))
        {
            std::cerr << "[NetClient] Failed to read protocol version\n";
            return;
        }

        uint8_t rawType = 0;
        if (!reader.Read<uint8_t>(rawType))
        {
            std::cerr << "[NetClient] Failed to read packet type\n";
            return;
        }

        Net::PacketType type = static_cast<Net::PacketType>(rawType);

        if (onPacket)
        {
            onPacket(data, len, type);
        }

        lastHeard_ = std::chrono::steady_clock::now();
    }

    void NetClient::HandleDisconnect(ENetEvent& event)
    {
        connected_ = false;
        std::cout << "[NetClient] Disconnected from server\n";

        if (onDisconnected) onDisconnected();
    }

    bool NetClient::ValidateIncomingPacket(const uint8_t* data, size_t len) const
    {
        if (!data) return false;
        if (len == 0) return false;
        if (len > Net::NET_MAX_PACKET_SIZE)
        {
            std::cerr << "[NetClient] Incoming packet too large: " << len << "\n";
            return false;
        }

        if (len < (sizeof(uint16_t) + sizeof(uint8_t)))
        {
            std::cerr << "[NetClient] Packet too small for header\n";
            return false;
        }

        uint8_t rawType = data[sizeof(uint16_t)];
        if (rawType > static_cast<uint8_t>(Net::PacketType::Disconnect))
        {
            std::cerr << "[NetClient] Unknown packet type: " << static_cast<int>(rawType) << "\n";
            return false;
        }

        return true;
    }

    void NetClient::GetSendFlagsForType(Net::PacketType type, enet_uint32& outFlags, uint8_t& outChannel) const
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
