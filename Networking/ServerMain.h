// --- ServerMain.h ---
#pragma once

#include "Connection.h"
#include "../Threading/ThreadingSystem.h"

#include <boost/asio.hpp>

namespace BSE
{
    class DLL_EXPORT ServerMain
    {
    public:
        using ByteBuffer = Cerialization::ByteBuffer;
        using ClientId = uint64_t;

        struct ServerStats
        {
            uint32_t connectedClients = 0;
            uint64_t bytesSent = 0;
            uint64_t bytesReceived = 0;
        };

        ServerMain(ThreadingSystem& threadSystem, uint16_t port, const std::string& address = "0.0.0.0");
        ~ServerMain();

        void Start();
        void Stop();

        bool SendTo(ClientId id, uint32_t typeId, const ByteBuffer& payload);
        void Broadcast(uint32_t typeId, const ByteBuffer& payload);

        void SetOnClientMessage(std::function<void(ClientId, uint32_t, const ByteBuffer&)> cb) { m_onClientMessage = std::move(cb); }
        void SetOnClientConnected(std::function<void(ClientId)> cb) { m_onClientConnected = std::move(cb); }
        void SetOnClientDisconnected(std::function<void(ClientId)> cb) { m_onClientDisconnected = std::move(cb); }

        ServerStats GetStats() const;

    private:
        void DoAccept();
        void OnNewConnection(std::shared_ptr<Connection> conn);
        void RemoveClient(ClientId id);

    private:
        ThreadingSystem& m_threadSystem;
        uint16_t m_port;
        std::string m_address;

        boost::asio::io_context m_ioContext;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_workGuard;

        std::atomic<ClientId> m_nextClientId{1};
        std::unordered_map<ClientId, std::shared_ptr<Connection>> m_clients;
        mutable std::mutex m_clientsMutex;

        std::function<void(ClientId, uint32_t, const ByteBuffer&)> m_onClientMessage;
        std::function<void(ClientId)> m_onClientConnected;
        std::function<void(ClientId)> m_onClientDisconnected;
    };
}
