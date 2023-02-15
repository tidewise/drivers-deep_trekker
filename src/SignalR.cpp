#include <base-logging/Logging.hpp>
#include <deep_trekker/SignalR.hpp>

#include <base/Time.hpp>

#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include <functional>
#include <stdexcept>

using namespace deep_trekker;
using namespace rtc;
using namespace std;

SignalR::SignalR(rtc::WebSocket::Configuration const& config,
    string const& host,
    string const& rock_peer_id,
    string const& deep_trekker_peer_id,
    bool curl_verbose,
    base::Time const& timeout)
    : m_ws(config, "deep-trekker")
    , m_host(host)
    , m_rock_peer_id(rock_peer_id)
    , m_deep_trekker_peer_id(deep_trekker_peer_id)
    , m_timeout(timeout)
{
    negotiate(curl_verbose);
    open();
}

SignalR::~SignalR()
{
    if (m_state == STATE_READY) {
        sessionLeave();
    }

    try {
        LOG_INFO_S << "signalr: closing websocket";
        m_ws.close(m_timeout);
    }
    catch (std::exception& e) {
        LOG_ERROR_S << "signalr: failed to close websocket: " << e.what();
    }
}

void SignalR::negotiate(bool curl_verbose)
{
    // Http post request
    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    string url_https = "https://" + m_host + "/sessionHub/negotiate?negotiateVersion=1";
    request.setOpt(new curlpp::options::Url(url_https));
    request.setOpt(new curlpp::options::Verbose(curl_verbose));
    list<string> header;
    header.push_back("Content-Type: application/json");
    request.setOpt(new curlpp::options::HttpHeader(header));
    request.setOpt(new curlpp::options::SslVerifyHost(false));
    request.setOpt(new curlpp::options::SslVerifyPeer(false));
    request.setOpt(new curlpp::options::PostFields(""));
    request.setOpt(new curlpp::options::PostFieldSize(0));
    std::ostringstream os;
    curlpp::options::WriteStream ws(&os);
    request.setOpt(ws);
    request.perform();
    os << request;

    string body = os.str().c_str();

    Json::Value json = m_ws.jsonParse(body);
    m_token = json["connectionToken"].asString();
    LOG_DEBUG_S << "Connection token: " << m_token << std::endl;
}

void SignalR::open()
{
    m_state = STATE_PENDING;

    m_ws.open("wss://" + m_host + "/sessionHub?id=" + m_token, m_timeout);
    m_ws.onJSONMessage([&](Json::Value const& data) { process(data); });
    m_ws.onJSONError([&](std::string const& msg) {
        LOG_ERROR_S << "error processing received JSON: " << msg << endl;
        setState(STATE_JSON_ERROR);
    });

    m_ws.onWebSocketError([&](string const& error) { setState(STATE_CONNECTION_LOST); });
}

void SignalR::start()
{
    LOG_INFO_S << "signalr: starting handshake";
    waitState(
        STATE_SESSION_CHECK,
        [&] { handshake(); },
        m_timeout);
}

void SignalR::setState(States state)
{
    unique_lock lock(m_state_lock);
    if (m_state != state) {
        LOG_DEBUG_S << "signalr: state change " << m_state << " -> " << state;
    }
    m_state = state;
    m_state_wait.notify_all();
}

void SignalR::waitState(States state, base::Time const& timeout)
{
    unique_lock lock(m_state_lock);
    waitState(state, lock, m_timeout);
}

void SignalR::waitState(States state, std::function<void()> f, base::Time const& timeout)
{
    unique_lock lock(m_state_lock);
    f();
    waitState(state, lock, timeout);
}

template <typename Lock>
void SignalR::waitState(States state, Lock& lock, base::Time const& timeout)
{
    while (m_state != state) {
        if (m_state_wait.wait_for(lock, chrono::microseconds(timeout.toMicroseconds())) ==
            cv_status::timeout) {
            throw std::runtime_error("timed out waiting for state " + to_string(state));
        }

        if (m_state >= STATE_FATAL_ERRORS) {
            throw runtime_error(
                "SignalR entered fatal error state " + to_string(m_state));
        }
    }
}

void SignalR::process(Json::Value const& msg)
{
    if (m_state == STATE_HANDSHAKE) {
        if (msg.empty()) {
            LOG_INFO_S << "signalr: received handshake reply";
            sessionCheck();
            setState(STATE_SESSION_CHECK);
        }
        else {
            LOG_ERROR_S
                << "expected empty message in reply to handshake message, but got "
                << m_ws.jsonToString(msg) << endl;
            setState(STATE_PROTOCOL_ERROR);
        }
        return;
    }

    if (!msg.isMember("type")) {
        LOG_ERROR_S << "received message from SignalR without a 'type' field: "
                    << m_ws.jsonToString(msg) << endl;
        setState(STATE_PROTOCOL_ERROR);
        return;
    }

    auto listener = m_listener.lock();
    if (!listener) {
        return;
    }

    int type = msg["type"].asInt();
    if (type == 6) {
        listener->pong();
    }
    else if (type == 3) {
        processReply(msg);
    }
    else if (type == 1 && msg["target"].asString() == "session_list") {
        m_session_id = msg["arguments"][0][0]["session_id"].asString();
        LOG_INFO_S << "signalr: session ID is " << m_session_id;
    }
    else if (type == 1 && msg["target"].asString() == "session_info") {
        bool found = false;
        auto clients = msg["arguments"][0]["clients"];
        for (unsigned int i = 0; !found && (i < clients.size()); ++i) {
            found = (clients[i]["client_id"] == m_rock_peer_id);
        }
        if (found) {
            LOG_INFO_S << "signalr: session joined";
        }
        else {
            LOG_ERROR_S << "signalr: session not joined";
            LOG_ERROR_S << "signalr: received session info " << msg;
            setState(STATE_PROTOCOL_ERROR);
            return;
        }
    }
    else if (type == 1 && msg["target"].asString() == "offer") {
        auto data = m_ws.jsonParse(msg["arguments"][0].asString());

        m_signalr_context["target"] = data["caller"];
        m_signalr_context["caller"] = data["target"];
        m_signalr_context["sessionId"] = data["sessionId"];
        listener->publishDescription(data["sdp"]["type"].asString(),
            data["sdp"]["sdp"].asString());
    }
    else if (type == 1 && msg["target"].asString() == "ice_candidate") {
        auto data = m_ws.jsonParse(msg["arguments"][0].asString());

        listener->publishICECandidate(data["candidate"]["content"].asString(),
            data["candidate"]["sdpMid"].asString());
    }

    switch (m_state) {
        case STATE_SESSION_CHECK: {
            if (!hasReceivedReply()) {
                LOG_DEBUG_S << "signalr: waiting for session check reply";
                return;
            }
            if (m_session_id.empty()) {
                LOG_DEBUG_S << "signalr: waiting for session ID";
                return;
            }

            sessionJoin();
            break;
        }

        case STATE_SESSION_JOIN: {
            if (!hasReceivedReply()) {
                return;
            }

            setState(STATE_READY);
            break;
        }

        default:
            break;
    }
}

string SignalR::getNextInvocationID()
{
    if (m_last_used_invocation_id != m_last_received_invocation_id) {
        throw runtime_error(
            "attempting to perform a call before the previous one was resolved");
    }
    return to_string(++m_last_used_invocation_id);
}

void SignalR::call(string const& target, Json::Value const& arg)
{
    Json::Value message;
    message["arguments"].append(m_ws.jsonToString(arg));
    message["target"] = target;
    message["type"] = 1;
    if (!hasReceivedReply()) {
        m_message_queue.push(message);
    }
    else {
        invoke(message);
    }
}

void SignalR::invoke(Json::Value message)
{

    message["invocationId"] = getNextInvocationID();
    auto msg = m_ws.jsonToString(message);
    m_ws.send(msg + "\x1e");
}

bool SignalR::hasReceivedReply() const
{
    return m_last_received_invocation_id == m_last_used_invocation_id;
}

Json::Value SignalR::processReply(Json::Value const& ret)
{
    if (ret["invocationId"].asString() != to_string(m_last_used_invocation_id)) {
        throw runtime_error("expected to receive result for call ID " +
                            to_string(m_last_used_invocation_id) + " but is " +
                            to_string(m_last_received_invocation_id));
    }

    m_last_received_invocation_id = m_last_used_invocation_id;
    if (!m_message_queue.empty()) {
        auto json = m_message_queue.front();
        m_message_queue.pop();
        invoke(json);
    }

    if (ret.isMember("error")) {
        throw runtime_error("received error in reply to call " +
                            to_string(m_last_used_invocation_id) + ": " +
                            ret["error"].asString());
    }

    return ret["result"];
}

void SignalR::handshake()
{
    Json::Value message;
    message["protocol"] = "json";
    message["version"] = 1;
    m_state = STATE_HANDSHAKE;
    m_ws.send(m_ws.jsonToString(message) + "\x1e");
}

void SignalR::sessionCheck()
{
    Json::Value args;
    args["client_id"] = m_rock_peer_id;
    m_state = STATE_SESSION_CHECK;
    LOG_INFO_S << "signalr: session check";
    call("session_check", args);
}

void SignalR::sessionJoin()
{
    if (m_session_id.empty()) {
        throw logic_error("sessionJoin called before the session ID is known");
    }

    Json::Value args;
    args["client_id"] = m_rock_peer_id;
    args["session_id"] = m_session_id;
    m_state = STATE_SESSION_JOIN;
    LOG_INFO_S << "signalr: starting session join";
    call("join_session", args);
}

void SignalR::sessionLeave()
{
    if (m_session_id.empty()) {
        throw logic_error("leaveSession called before the it was joined");
    }

    Json::Value args;
    args["client_id"] = m_rock_peer_id;
    args["session_id"] = m_session_id;
    m_state = STATE_SESSION_LEAVE;
    LOG_INFO_S << "signalr: starting session leave";
    call("leave_session", args);
}

void SignalR::publishICECandidate(std::string const& candidate, std::string const& mid)
{
    Json::Value msg = m_signalr_context;

    Json::Value data;
    data["sdpMid"] = mid;
    data["sdpMLineIndex"] = 0;
    data["content"] = candidate.substr(2);
    msg["candidate"] = data;
    call("ice_candidate", msg);
}
void SignalR::publishDescription(std::string const& type, std::string const& sdp)
{
    Json::Value msg = m_signalr_context;

    Json::Value sdp_message;
    sdp_message["type"] = type;
    sdp_message["sdp"] = sdp;
    msg["sdp"] = sdp_message;
    call(type, msg);
}

void SignalR::ping()
{
    string msg("{type: 6}\x1e");
    m_ws.send(msg);
}

void SignalR::pong()
{
    string msg("{type: 6}\x1e");
    m_ws.send(msg);
}

void SignalR::setListener(shared_ptr<WebRTCNegotiationInterface> listener)
{
    m_listener = listener;
}