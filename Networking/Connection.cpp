// Connection.cpp
#include "Connection.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace BSE
{
    using boost::asio::ip::tcp;

    static void PackHeader(const Connection::MessageHeader& h, std::vector<uint8_t>& out)
    {
        out.resize(sizeof(Connection::MessageHeader));
        std::memcpy(out.data(), &h, sizeof(h));
    }

    static Connection::MessageHeader UnpackHeader(const uint8_t* data)
    {
        Connection::MessageHeader h;
        std::memcpy(&h, data, sizeof(h));
        return h;
    }

    Connection::Connection(boost::asio::ip::tcp::socket socket)
        : m_socket(std::make_unique<tcp::socket>(std::move(socket))),
          m_executor(m_socket->get_executor()),
          m_strand(m_executor)
    {}

    Connection::Connection(boost::asio::io_context& ctx)
        : m_socket(std::make_unique<tcp::socket>(ctx)),
          m_executor(ctx.get_executor()),
          m_strand(m_executor)
    {}

    Connection::~Connection()
    {
        Close();
    }

    void Connection::AsyncConnect(const std::string& host, uint16_t port, std::function<void(const boost::system::error_code&)> onConnected)
    {
        tcp::resolver resolver(m_executor);
        auto self = shared_from_this();

        resolver.async_resolve(host, std::to_string(port),
            [this, self, onConnected](const boost::system::error_code& ec, tcp::resolver::results_type results)
            {
                if (ec)
                {
                    if (onConnected) onConnected(ec);
                    if (m_onError) m_onError(ec);
                    return;
                }

                boost::asio::async_connect(*m_socket, results,
                    [this, self, onConnected](const boost::system::error_code& ec, const tcp::endpoint&)
                    {
                        if (onConnected) onConnected(ec);
                        if (!ec)
                        {
                            Start();
                        }
                        else
                        {
                            if (m_onError) m_onError(ec);
                        }
                    });
            });
    }

    void Connection::Start()
    {
        BeginReadHeader();
    }

    void Connection::Close()
    {
        boost::system::error_code ec;
        if (m_socket && m_socket->is_open())
        {
            m_socket->shutdown(tcp::socket::shutdown_both, ec);
            m_socket->close(ec);
        }

        if (m_onClose) m_onClose();
    }

    void Connection::Send(uint32_t typeId, const ByteBuffer& payload)
    {
        MessageHeader h;
        h.protocolVersion = 1;
        h.typeId = typeId;
        h.payloadSize = static_cast<uint32_t>(payload.size());

        std::vector<uint8_t> framed;
        PackHeader(h, framed);
        framed.insert(framed.end(), payload.begin(), payload.end());

        {
            std::lock_guard<std::mutex> lock(m_outgoingMutex);
            m_outgoing.emplace_back(std::move(framed));
            if (!m_writeInProgress)
            {
                m_writeInProgress = true;
                auto self = shared_from_this();
                boost::asio::post(m_strand, [this, self]() { DoWriteNext(); });
            }
        }
    }

    void Connection::DoWriteNext()
    {
        ByteBuffer toSend;
        {
            std::lock_guard<std::mutex> lock(m_outgoingMutex);
            if (m_outgoing.empty())
            {
                m_writeInProgress = false;
                return;
            }
            toSend = std::move(m_outgoing.front());
            m_outgoing.pop_front();
        }

        auto self = shared_from_this();
        boost::asio::async_write(*m_socket, boost::asio::buffer(toSend),
            boost::asio::bind_executor(m_strand,
                [this, self, bytes = toSend.size()](const boost::system::error_code& ec, std::size_t written)
                {
                    if (!ec)
                    {
                        m_bytesSent += written;
                        DoWriteNext();
                    }
                    else
                    {
                        if (m_onError) m_onError(ec);
                        Close();
                    }
                }));
    }

    void Connection::BeginReadHeader()
    {
        auto self = shared_from_this();
        boost::asio::async_read(*m_socket, boost::asio::buffer(m_headerBuf),
            boost::asio::bind_executor(m_strand,
                [this, self](const boost::system::error_code& ec, std::size_t bytes)
                {
                    if (ec)
                    {
                        if (m_onError) m_onError(ec);
                        Close();
                        return;
                    }

                    if (bytes != sizeof(MessageHeader))
                    {
                        if (m_onError) m_onError(boost::asio::error::operation_aborted);
                        Close();
                        return;
                    }

                    MessageHeader h = UnpackHeader(m_headerBuf.data());
                    constexpr uint32_t MAX_PAYLOAD = 10 * 1024 * 1024;
                    if (h.payloadSize > MAX_PAYLOAD)
                    {
                        if (m_onError) m_onError(boost::asio::error::message_size);
                        Close();
                        return;
                    }

                    BeginReadPayload(h);
                }));
    }

    void Connection::BeginReadPayload(MessageHeader header)
    {
        m_payloadBuf.resize(header.payloadSize);
        auto self = shared_from_this();

        if (header.payloadSize == 0)
        {
            m_bytesReceived += sizeof(MessageHeader);
            if (m_onMessage) m_onMessage(header.typeId, m_payloadBuf);
            BeginReadHeader();
            return;
        }

        boost::asio::async_read(*m_socket, boost::asio::buffer(m_payloadBuf),
            boost::asio::bind_executor(m_strand,
                [this, self, header](const boost::system::error_code& ec, std::size_t bytes)
                {
                    if (ec)
                    {
                        if (m_onError) m_onError(ec);
                        Close();
                        return;
                    }

                    m_bytesReceived += sizeof(MessageHeader) + bytes;
                    if (m_onMessage) m_onMessage(header.typeId, m_payloadBuf);
                    BeginReadHeader();
                }));
    }
}
