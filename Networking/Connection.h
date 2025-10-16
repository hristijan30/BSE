#pragma once

#include "Cerialization.h"
#include <boost/asio.hpp>
#include <deque>

namespace BSE
{
    class Connection : public std::enable_shared_from_this<Connection>
    {
    public:
        using ByteBuffer = Cerialization::ByteBuffer;
        using OnMessageFn = std::function<void(uint32_t typeId, const ByteBuffer& payload)>;
        using OnCloseFn = std::function<void()>;
        using OnErrorFn = std::function<void(const boost::system::error_code&)>;

        struct MessageHeader
        {
            uint16_t protocolVersion = 1;
            uint32_t typeId = 0;
            uint32_t payloadSize = 0;
        };

        Connection(boost::asio::ip::tcp::socket socket);
        Connection(boost::asio::io_context& ctx);
        ~Connection();

        void AsyncConnect(const std::string& host, uint16_t port, std::function<void(const boost::system::error_code&)> onConnected);

        void Start();
        void Close();

        void Send(uint32_t typeId, const ByteBuffer& payload);

        template<typename T>
        void SendTyped(uint32_t typeId, const T& obj)
        {
            ByteBuffer b = Cerialization::Instance().Serialize<T>(typeId, obj);
            Send(typeId, b);
        }

        void SetOnMessage(OnMessageFn cb) { m_onMessage = std::move(cb); }
        void SetOnClose(OnCloseFn cb) { m_onClose = std::move(cb); }
        void SetOnError(OnErrorFn cb) { m_onError = std::move(cb); }

        uint64_t GetBytesSent() const { return m_bytesSent.load(); }
        uint64_t GetBytesReceived() const { return m_bytesReceived.load(); }

        bool IsOpen() const { return m_socket && m_socket->is_open(); }

    private:
        void BeginReadHeader();
        void BeginReadPayload(MessageHeader header);
        void DoWriteNext();

    private:
        boost::asio::any_io_executor m_executor;
        std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
        boost::asio::strand<boost::asio::any_io_executor> m_strand;

        std::deque<ByteBuffer> m_outgoing;
        std::mutex m_outgoingMutex;
        bool m_writeInProgress = false;

        std::array<uint8_t, sizeof(MessageHeader)> m_headerBuf{};
        ByteBuffer m_payloadBuf;

        OnMessageFn m_onMessage;
        OnCloseFn m_onClose;
        OnErrorFn m_onError;

        std::atomic<uint64_t> m_bytesSent{0};
        std::atomic<uint64_t> m_bytesReceived{0};
    };
}
