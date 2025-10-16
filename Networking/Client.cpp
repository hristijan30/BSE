#include "Client.h"

namespace BSE
{
    using boost::asio::ip::tcp;

    Client::Client(ThreadingSystem& threadSystem)
        : m_threadSystem(threadSystem),
          m_ioContext(),
          m_workGuard(boost::asio::make_work_guard(m_ioContext))
    {
        StartIoContext();
    }

    Client::~Client()
    {
        Disconnect();
        StopIoContext();
    }

    void Client::StartIoContext()
    {
        m_threadSystem.SubmitTask([this]() {
            try { m_ioContext.run(); }
            catch (const std::exception& e) { std::cerr << "Client: io_context.run exception: " << e.what() << std::endl; }
        });
    }

    void Client::StopIoContext()
    {
        m_workGuard.reset();
        m_ioContext.stop();
    }

    void Client::Connect(const std::string& host, uint16_t port)
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (m_connected) return;

        m_host = host;
        m_port = port;

        m_connection = std::make_unique<Connection>(m_ioContext);
        auto self = this;

        m_connection->SetOnMessage([this](uint32_t typeId, const ByteBuffer& payload) {
            OnMessageInternal(typeId, payload);
        });

        m_connection->SetOnClose([this]() {
            OnDisconnectedInternal();
        });

        m_connection->SetOnError([this](const boost::system::error_code& ec) {
            std::cerr << "Client: connection error: " << ec.message() << std::endl;
        });

        m_connection->AsyncConnect(host, port, [this](const boost::system::error_code& ec) {
            OnConnectedInternal(ec);
        });
    }

    void Client::Disconnect()
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (m_connection)
        {
            m_connection->Close();
            m_connection.reset();
        }
        m_connected = false;
        FailPendingRequests();
    }

    bool Client::IsConnected() const
    {
        return m_connected.load();
    }

    void Client::OnConnectedInternal(const boost::system::error_code& ec)
    {
        if (ec)
        {
            m_connected = false;
            if (m_onConnected) m_onConnected(); // notify even on failure? you may change this behavior
            return;
        }

        m_connected = true;
        if (m_onConnected) m_onConnected();
    }

    void Client::OnDisconnectedInternal()
    {
        m_connected = false;
        if (m_onDisconnected) m_onDisconnected();
        FailPendingRequests();
    }

    void Client::Send(uint32_t typeId, const ByteBuffer& payload)
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (!m_connection || !m_connection->IsOpen()) return;
        m_connection->Send(typeId, payload);
    }

    Client::RequestId Client::NextRequestId()
    {
        return m_nextRequestId.fetch_add(1);
    }

    Client::RequestId Client::AsyncRequest(uint32_t requestTypeId, uint32_t responseTypeId, const ByteBuffer& payload, RequestCallback cb, uint32_t timeoutMs)
    {
        if (!IsConnected())
        {
            if (cb) cb(false, {});
            return 0;
        }

        RequestId reqId = NextRequestId();

        // build payload = [uint32_t reqId] + payload
        ByteBuffer framed;
        AppendPod(framed, reqId);
        framed.insert(framed.end(), payload.begin(), payload.end());

        // store pending
        auto timer = std::make_shared<boost::asio::steady_timer>(m_ioContext);
        PendingReq p;
        p.cb = cb;
        p.responseType = responseTypeId;
        p.timer = timer;

        {
            std::lock_guard<std::mutex> lock(m_pendingMutex);
            m_pendingRequests.emplace(reqId, std::move(p));
        }

        // set timeout
        timer->expires_after(std::chrono::milliseconds(timeoutMs));
        timer->async_wait([this, reqId](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted) return; // cancelled
            PendingReq pr;
            {
                std::lock_guard<std::mutex> lock(m_pendingMutex);
                auto it = m_pendingRequests.find(reqId);
                if (it == m_pendingRequests.end()) return;
                pr = it->second;
                m_pendingRequests.erase(it);
            }
            if (pr.cb) pr.cb(false, {});
        });

        // send
        Send(requestTypeId, framed);
        return reqId;
    }

    Client::RequestId Client::Ping(RequestCallback cb, uint32_t timeoutMs)
    {
        // Ping payload: [uint64_t timestamp_ms]
        uint64_t now = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        ByteBuffer payload;
        AppendPod(payload, now);
        return AsyncRequest(m_pingType, m_pongType, payload, [cb, now](bool success, const ByteBuffer& resp) {
            if (!success)
            {
                if (cb) cb(false, {});
                return;
            }
            // response expected: [uint32_t reqId][uint64_t serverTimestamp_ms]
            if (resp.size() < sizeof(uint64_t)) { if (cb) cb(false, {}); return; }
            uint64_t serverTs = ReadPod<uint64_t>(resp, 0);
            // roundtrip = now -> serverTs? We don't compute exact RTT here since we don't have receive timestamp
            if (cb) cb(true, resp);
        }, timeoutMs);
    }

    void Client::OnMessageInternal(uint32_t typeId, const ByteBuffer& payload)
    {
        // If payload has at least 4 bytes, it may be a response to a pending request
        if (payload.size() >= sizeof(RequestId))
        {
            RequestId rid = ReadPod<RequestId>(payload, 0);
            std::lock_guard<std::mutex> lock(m_pendingMutex);
            auto it = m_pendingRequests.find(rid);
            if (it != m_pendingRequests.end() && it->second.responseType == typeId)
            {
                auto cb = it->second.cb;
                // cancel timer
                if (it->second.timer) it->second.timer->cancel();
                // extract response payload (skip requestId)
                ByteBuffer resp(payload.begin() + sizeof(RequestId), payload.end());
                m_pendingRequests.erase(it);
                if (cb) cb(true, resp);
                return;
            }
        }

        // otherwise deliver as general message
        if (m_onMessage) m_onMessage(typeId, payload);
    }

    void Client::FailPendingRequests()
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);
        for (auto& kv : m_pendingRequests)
        {
            if (kv.second.cb) kv.second.cb(false, {});
            if (kv.second.timer) kv.second.timer->cancel();
        }
        m_pendingRequests.clear();
    }

    uint64_t Client::GetBytesSent() const
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (!m_connection) return 0;
        return m_connection->GetBytesSent();
    }

    uint64_t Client::GetBytesReceived() const
    {
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        if (!m_connection) return 0;
        return m_connection->GetBytesReceived();
    }
}
