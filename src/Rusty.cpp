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
    m_ws.send(m_ws.jsonToString(data));
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
    data["mid"] = m_ws.jsonParse(candidate)["mid"];
    msg["data"] = data;
    m_ws.send(m_ws.jsonToString(data));
}