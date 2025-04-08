#pragma once
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

class Session :
    public std::enable_shared_from_this<Session>
{
public:
    using HandlerType = std::function<void(std::shared_ptr<Session>, const nlohmann::json&)>;

public:
    Session(int socketFd, HandlerType handler);
    Session(const Session&) = delete;
    ~Session();

public:
    void Start();
    void Abort() { m_bAbort = true; }
    void Send(const nlohmann::json& json);

private:
    void Process();

private:
    int m_iSocketFd = -1;
    bool m_bAbort = false;
    HandlerType m_stHandler;
    std::vector<uint8_t> m_stBuffer;
};

using SessionPtr = std::shared_ptr<Session>;
