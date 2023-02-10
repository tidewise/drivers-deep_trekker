#include <base-logging/Logging.hpp>
#include <deep_trekker/Rusty.hpp>

using namespace deep_trekker;
using namespace rtc;

Rusty::Rusty(WebSocket::Configuration const& config,
    string const& host,
    string const& rock_peer_id,
    string const& deep_trekker_peer_id,
    base::Time const& timeout)
    : m_ws(config, "rock")
    , m_host(host)
    , m_rock_peer_id(rock_peer_id)
    , m_deep_trekker_peer_id(deep_trekker_peer_id)
    , m_timeout(timeout)
{
    open();
}

Rusty::~Rusty()
{
    try {
        LOG_INFO_S << "rusty: closing websocket";
        m_ws.close(m_timeout);
    }
    catch (std::exception& e) {
        LOG_ERROR_S << "rusty: failed to close websocket " << e.what();
    }
}

void Rusty::open()
{
    m_ws.open("ws://" + m_host + "?user=" + m_deep_trekker_peer_id, m_timeout);
    m_ws.onJSONMessage([&](Json::Value const& msg) {
        auto action = msg["action"].asString();
        if (action == "open") {
            {
                std::unique_lock lock(m_on_new_client_lock);
                m_has_new_client = true;
                m_client.reset();
                m_on_new_client.notify_all();
            }
        }

        auto client = m_client.lock();
        if (!client) {
            return;
        }

        if (action == "ping") {
            pong();
        }
        else if (action == "offer" || action == "answer") {
            client->publishDescription(action, msg["data"]["description"].asString());
        }
        else if (action == "candidate") {
            client->publishICECandidate(msg["data"]["candidate"].asString(),
                msg["data"]["mid"].asString());
        }
    });
}

void Rusty::waitNewClient()
{
    std::unique_lock lock(m_on_new_client_lock);
    while (!m_has_new_client) {
        m_on_new_client.wait(lock);
    }
    m_has_new_client = false;
}

bool Rusty::setClient(std::shared_ptr<WebRTCNegotiationInterface> client)
{
    std::unique_lock lock(m_on_new_client_lock);
    if (m_has_new_client) {
        return false;
    }
    m_client = client;
    return true;
}

void Rusty::sendPingPong(std::string const& type)
{
    Json::Value msg;
    msg["protocol"] = "one-to-one";
    msg["to"] = m_rock_peer_id;
    msg["action"] = type;

    Json::Value data;
    data["from"] = m_deep_trekker_peer_id;
    msg["data"] = data;
    m_ws.send(msg);
}

void Rusty::ping()
{
    sendPingPong("ping");
}

void Rusty::pong()
{
    sendPingPong("pong");
}

void Rusty::publishDescription(std::string const& type, std::string const& sdp)
{
    Json::Value msg;
    msg["protocol"] = "one-to-one";
    msg["to"] = m_rock_peer_id;
    msg["action"] = type;

    Json::Value data;
    data["from"] = m_deep_trekker_peer_id;
    data["description"] = sdp;

    msg["data"] = data;
    m_ws.send(m_ws.jsonToString(msg));
}

void Rusty::publishICECandidate(std::string const& candidate)
{
    Json::Value msg;
    msg["protocol"] = "one-to-one";
    msg["to"] = m_rock_peer_id;
    msg["action"] = "candidate";

    Json::Value data;
    data["from"] = m_deep_trekker_peer_id;
    data["candidate"] = candidate;
    data["mid"] = mid;
    msg["data"] = data;
    m_ws.send(m_ws.jsonToString(msg));
}