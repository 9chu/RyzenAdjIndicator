#include "Session.hpp"

#include <cassert>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;

Session::Session(int socketFd, HandlerType handler)
    : m_iSocketFd(socketFd), m_stHandler(std::move(handler))
{
}

Session::~Session()
{
    ::close(m_iSocketFd);
}

void Session::Start()
{
    std::thread([self = shared_from_this()]() mutable {
        self->Process();
    }).detach();
}

void Session::Send(const nlohmann::json& payload)
{
    if (m_bAbort)
        return;

    auto encoded = payload.dump();
    uint32_t length = static_cast<uint32_t>(encoded.size());
    ::send(m_iSocketFd, &length, sizeof(length), 0);
    ::send(m_iSocketFd, encoded.data(), length, 0);
}

void Session::Process()
{
    m_bAbort = false;
    while (!m_bAbort)
    {
        // 读四个字节的包长
        uint32_t length;
        ssize_t ret = ::recv(m_iSocketFd, &length, sizeof(length), 0);
        if (ret <= 0 || ret != sizeof(length))
        {
            fprintf(stderr, "recv length failed, ret: %ld, errno: %d\n", ret, errno);
            m_bAbort = true;
            continue;
        }

        if (length > 1024 * 1024)
        {
            fprintf(stderr, "recv length too long, length: %u\n", length);
            m_bAbort = true;
            continue;
        }
        m_stBuffer.resize(length);

        // 读包体
        ret = ::recv(m_iSocketFd, m_stBuffer.data(), length, 0);
        if (ret <= 0 || ret != length)
        {
            fprintf(stderr, "recv body failed, ret: %ld, errno: %d\n", ret, errno);
            m_bAbort = true;
            continue;
        }

        nlohmann::json json;
        try
        {
            json = nlohmann::json::parse(reinterpret_cast<char*>(m_stBuffer.data()),
                reinterpret_cast<char*>(m_stBuffer.data()) + m_stBuffer.size());
        }
        catch (const std::exception& e)
        {
            fprintf(stderr, "json parse failed, error: %s\n", e.what());
            m_bAbort = true;
            continue;
        }

        // 调用处理函数
        assert(m_stHandler);
        m_stHandler(shared_from_this(), json);
    }
}
