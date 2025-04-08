#pragma once
#include "Session.hpp"

class Server
{
    using HandlerType = Session::HandlerType;

public:
    Server(std::string socketPath);
    Server(const Server&) = delete;
    ~Server();

public:
    void Run();
    void Stop() { m_bNotifyStop = true; }
    void SetHandler(HandlerType handler) { m_stHandler = std::move(handler); }

private:
    std::string m_stSocketPath;
    bool m_bNotifyStop = false;
    int m_iSocketFd = -1;
    HandlerType m_stHandler;
};
