#pragma once

#include "Connection.h"
#include "Cerialization.h"
#include "../Threading/ThreadingSystem.h"

#include <boost/asio.hpp>

namespace BSE
{
    class DLL_EXPORT Client
    {
    public:
        using ByteBuffer = Cerialization::ByteBuffer;
        using RequestId = uint32_t;

        using RequestCallback = std::function<void(bool, const ByteBuffer&)>;
        using MessageCallback = std::function<void(uint32_t typeId, const ByteBuffer&)>;
        using ConnectionCallback = std::function<void()>;

        static constexpr uint32_t DefaultPingType = 0xFFFF0001;
        static constexpr uint32_t DefaultPongType = 0xFFFF0002;

        Client(ThreadingSystem& threadSystem);
        ~Client();

        void Connect(const std::string& host, uint16_t port);
        void Disconnect();

        bool IsConnected() const;

        void SetOnConnected(ConnectionCallback cb) { m_onConnected = std::move(cb); }
        void SetOnDisconnected(ConnectionCallback cb) { m_onDisconnected = std::move(cb); }
        void SetOnMessage(MessageCallback cb) { m_onMessage = std::move(cb); }

        void Send(uint32_t typeId, const ByteBuffer& payload);
        template<typename T>
        void SendTyped(uint32_t typeId, const T& obj)
        {
            ByteBuffer b = Cerialization::Instance().Serialize<T>(typeId, obj);
            Send(typeId, b);
        }

        RequestId AsyncRequest(uint32_t requestTypeId, uint32_t responseTypeId, const ByteBuffer& payload,
                               RequestCallback cb, uint32_t timeoutMs = 5000);
        RequestId Ping(RequestCallback cb, uint32_t timeoutMs = 5000);

        uint64_t GetBytesSent() const;
        uint64_t GetBytesReceived() const;

    private:
        void StartIoContext();
        void StopIoContext();

        void OnConnectedInternal(const boost::system::error_code& ec);
        void OnMessageInternal(uint32_t typeId, const ByteBuffer& payload);
        void OnDisconnectedInternal();

        RequestId NextRequestId();
        void FailPendingRequests();

    private:
        ThreadingSystem& m_threadSystem;

        boost::asio::io_context m_ioContext;
        std::unique_ptr<Connection> m_connection;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_workGuard;

        std::string m_host;
        uint16_t m_port = 0;

        std::atomic<bool> m_connected{false};
        mutable std::mutex m_connectionMutex;

        struct PendingReq
        {
            RequestCallback cb;
            uint32_t responseType;
            std::shared_ptr<boost::asio::steady_timer> timer;
        };

        mutable std::mutex m_pendingMutex;
        std::unordered_map<RequestId, PendingReq> m_pendingRequests;
        std::atomic<RequestId> m_nextRequestId{1};

        ConnectionCallback m_onConnected;
        ConnectionCallback m_onDisconnected;
        MessageCallback m_onMessage;

        uint32_t m_pingType = DefaultPingType;
        uint32_t m_pongType = DefaultPongType;
    };
}
