#include <base-logging/Logging.hpp>
#include <deep_trekker/Rusty.hpp>

using namespace deep_trekker;
using namespace rtc;
using namespace std;
using namespace base;

Rusty::Rusty(WebSocket::Configuration const& config,
    string const& host,
    string const& rock_peer_id,
    string const& deep_trekker_peer_id,
    base::Time const& timeout,
    base::Time const& client_ping_timeout)
    : m_ws(config, "rock")
    , m_host(host)
    , m_rock_peer_id(rock_peer_id)
    , m_deep_trekker_peer_id(deep_trekker_peer_id)
    , m_timeout(timeout)
    , m_client_ping_timeout(client_ping_timeout)
{
    open();
}

Rusty::~Rusty()
{
    try {
        LOG_INFO_S << "rusty: closing websocket";
        m_ws.close(m_timeout);
    }
    catch (exception& e) {
        LOG_ERROR_S << "rusty: failed to close websocket " << e.what();
    }
}

void Rusty::open()
{
    m_ws.open("ws://" + m_host + "?user=" + m_deep_trekker_peer_id, m_timeout);
    m_ws.onJSONMessage([&](Json::Value const& msg) {
        auto action = msg["action"].asString();
        if (action == "request-offer") {
            {
                unique_lock lock(m_poll_lock);
                if (!m_client.lock()) {
                    m_has_new_client = true;
                }
            }
        }
        else if (action == "open") {
            {
                unique_lock lock(m_poll_lock);
                m_client.reset();
                m_has_new_client = true;
            }
        }

        auto client = m_client.lock();
        if (!client) {
            return;
        }

        if (!m_client_ping_timeout.isNull())
        {
            unique_lock lock(m_poll_lock);
            m_client_ping_deadline = Time::now() + m_client_ping_timeout;
        }

        if (action == "ping") {
            pong();
            client->ping();
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

void Rusty::waitClientNew()
{
    while (true) {
        unique_lock lock(m_poll_lock);
        if (m_has_new_client) {
            m_has_new_client = false;
            return;
        }

        lock.unlock();
        this_thread::sleep_for(100ms);
    }
}

void Rusty::waitClientEnd()
{
    while (true) {
        unique_lock lock(m_poll_lock);
        if (m_has_new_client) {
            return;
        }

        if (!m_client_ping_deadline.isNull() && Time::now() > m_client_ping_deadline) {
            LOG_ERROR_S << "Rusty client timed out, disconnecting";
            return;
        }

        lock.unlock();
        ping();
        this_thread::sleep_for(100ms);
    }
}

bool Rusty::setClient(shared_ptr<WebRTCNegotiationInterface> client)
{
    unique_lock lock(m_poll_lock);
    if (m_has_new_client) {
        return false;
    }
    m_client = client;
    return true;
}

void Rusty::sendPingPong(string const& type)
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

void Rusty::publishDescription(string const& type, string const& sdp)
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

void Rusty::publishICECandidate(string const& candidate, string const& mid)
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
