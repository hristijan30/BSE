#include "ServerMain.h"

namespace BSE
{
    using boost::asio::ip::tcp;

    ServerMain::ServerMain(ThreadingSystem& threadSystem, uint16_t port, const std::string& address)
        : m_threadSystem(threadSystem),
          m_port(port),
          m_address(address),
          m_ioContext(),
          m_acceptor(nullptr),
          m_workGuard(boost::asio::make_work_guard(m_ioContext))
    {
    }

    ServerMain::~ServerMain()
    {
        Stop();
    }

    void ServerMain::Start()
    {
        boost::system::error_code ec;
        tcp::endpoint endpoint(boost::asio::ip::make_address(m_address, ec), m_port);
        if (ec)
        {
            throw std::runtime_error(std::string("ServerMain: invalid address: ") + ec.message());
        }

        m_acceptor = std::make_unique<tcp::acceptor>(m_ioContext);
        m_acceptor->open(endpoint.protocol(), ec);
        if (ec) throw std::runtime_error(std::string("ServerMain: acceptor open failed: ") + ec.message());

        m_acceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
        m_acceptor->bind(endpoint, ec);
        if (ec) throw std::runtime_error(std::string("ServerMain: bind failed: ") + ec.message());

        m_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) throw std::runtime_error(std::string("ServerMain: listen failed: ") + ec.message());

        DoAccept();

        m_threadSystem.SubmitTask([this]() {
            try {
                m_ioContext.run();
            }
            catch (const std::exception& e) {
                std::cerr << "ServerMain: io_context.run exception: " << e.what() << std::endl;
            }
        });
    }

    void ServerMain::Stop()
    {
        boost::system::error_code ec;
        if (m_acceptor)
        {
            m_acceptor->close(ec);
            m_acceptor.reset();
        }

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (auto& kv : m_clients)
            {
                if (kv.second) kv.second->Close();
            }
            m_clients.clear();
        }

        m_ioContext.stop();
        m_workGuard.reset();
    }

    void ServerMain::DoAccept()
    {
        if (!m_acceptor) return;

        auto socket = std::make_shared<tcp::socket>(m_ioContext);
        m_acceptor->async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
            if (!ec)
            {
                try
                {
                    auto conn = std::make_shared<Connection>(std::move(*socket));
                    OnNewConnection(conn);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ServerMain: failed to create Connection: " << e.what() << std::endl;
                }
            }
            else
            {
                std::cerr << "ServerMain: accept error: " << ec.message() << std::endl;
            }

            DoAccept();
        });
    }

    void ServerMain::OnNewConnection(std::shared_ptr<Connection> conn)
    {
        ClientId id = m_nextClientId.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients.emplace(id, conn);
        }

        conn->SetOnMessage([this, id](uint32_t typeId, const ByteBuffer& payload) {
            if (m_onClientMessage) m_onClientMessage(id, typeId, payload);
        });

        conn->SetOnClose([this, id]() {
            RemoveClient(id);
            if (m_onClientDisconnected) m_onClientDisconnected(id);
        });

        conn->SetOnError([this, id](const boost::system::error_code& ec) {
            std::cerr << "ServerMain: client " << id << " error: " << ec.message() << std::endl;
        });

        conn->Start();

        if (m_onClientConnected) m_onClientConnected(id);
    }

    void ServerMain::RemoveClient(ClientId id)
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        auto it = m_clients.find(id);
        if (it != m_clients.end())
        {
            if (it->second) it->second->Close();
            m_clients.erase(it);
        }
    }

    bool ServerMain::SendTo(ClientId id, uint32_t typeId, const ByteBuffer& payload)
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        auto it = m_clients.find(id);
        if (it == m_clients.end()) return false;
        if (!it->second->IsOpen()) return false;
        it->second->Send(typeId, payload);
        return true;
    }

    void ServerMain::Broadcast(uint32_t typeId, const ByteBuffer& payload)
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto& kv : m_clients)
        {
            if (kv.second && kv.second->IsOpen())
                kv.second->Send(typeId, payload);
        }
    }

    ServerMain::ServerStats ServerMain::GetStats() const
    {
        ServerStats s;
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        s.connectedClients = static_cast<uint32_t>(m_clients.size());
        for (const auto& kv : m_clients)
        {
            if (kv.second)
            {
                s.bytesSent += kv.second->GetBytesSent();
                s.bytesReceived += kv.second->GetBytesReceived();
            }
        }
        return s;
    }
}
