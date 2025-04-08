#include "Server.hpp"

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fmt/format.h>

using namespace std;

Server::Server(std::string socketPath)
    : m_stSocketPath(std::move(socketPath))
{
    if (m_stSocketPath.size() >= sizeof(sockaddr_un::sun_path))
        throw std::runtime_error("Socket path too long");

    struct sockaddr_un addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    ::strcpy(addr.sun_path, m_stSocketPath.c_str());

    ::umask(0);
    ::unlink(m_stSocketPath.c_str());
    m_iSocketFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_iSocketFd < 0)
        throw std::runtime_error(fmt::format("Failed to create socket, path: {}, errno: {}", m_stSocketPath, errno));

    auto ret = ::bind(m_iSocketFd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
    {
        ::close(m_iSocketFd);
        throw std::runtime_error(fmt::format("Failed to bind socket, path: {}, errno: {}", m_stSocketPath, errno));
    }

    ret = ::listen(m_iSocketFd, 5);
    if (ret != 0)
    {
        ::close(m_iSocketFd);
        throw std::runtime_error(fmt::format("Failed to listen socket, path: {}, errno: {}", m_stSocketPath, errno));
    }
}

Server::~Server()
{
    ::unlink(m_stSocketPath.c_str());
    ::close(m_iSocketFd);
}

void Server::Run()
{
    m_bNotifyStop = false;
    while (!m_bNotifyStop)
    {
        // 使用 select 检查 m_iSocketFd 是否有连接
        fd_set readFdSet;
        FD_ZERO(&readFdSet);
        FD_SET(m_iSocketFd, &readFdSet);
        timeval timeout {
            .tv_sec = 1,
            .tv_usec = 0
        };
        assert(m_iSocketFd >= 0 && m_iSocketFd < FD_SETSIZE);
        int ret = ::select(m_iSocketFd + 1, &readFdSet, nullptr, nullptr, &timeout);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error(fmt::format("Failed to select socket, errno: {}", errno));
        }
        if (ret == 0 || !FD_ISSET(m_iSocketFd, &readFdSet))
            continue;

        // 处理客户端请求
        auto clientFd = ::accept(m_iSocketFd, nullptr, nullptr);
        if (clientFd < 0)
            throw std::runtime_error(fmt::format("Failed to accept socket, errno: {}", errno));

        // 开启一个线程来处理客户端请求
        auto session = make_shared<Session>(clientFd, m_stHandler);
        session->Start();
    }
}
