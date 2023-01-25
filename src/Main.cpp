#include "rtc/rtc.hpp"
#include "json/json.h"
#include <base-logging/Logging.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <deep_trekker/HttpPostRequestReplyMessageDecoder.hpp>
#include <deep_trekker/RustySignalMessageDecoder.hpp>
#include <deep_trekker/SignalRMessageDecoder.hpp>
#include <future>
#include <iostream>
#include <sstream>

using namespace deep_trekker;
using namespace std;
using namespace rtc;
using std::shared_ptr;
using std::weak_ptr;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr)
{
    return ptr;
}
void pong(shared_ptr<rtc::WebSocket>& rusty_websocket,
    RustySignalMessageDecoder& decoder,
    string const& local_peer_id);
void sendInitialPayload(shared_ptr<rtc::WebSocket>& signalr_websocket);
void sendSessionCheck(shared_ptr<rtc::WebSocket>& signalr_websocket,
    string& local_peer_id);
void joinSession(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id);
void leaveSession(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id);
bool initialHandShakeFinalized(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id);
void offerAnswerRustyMessageParser(shared_ptr<rtc::WebSocket>& signalr_websocket,
    RustySignalMessageDecoder& decoder);
void candidateRustyMessageParser(shared_ptr<rtc::WebSocket>& signalr_websocket,
    RustySignalMessageDecoder& decoder);
void offerAnswerSignalMessageParser(shared_ptr<rtc::WebSocket>& rusty_websocket,
    SignalRMessageDecoder& decoder);
void candidateSignalMessageParser(shared_ptr<rtc::WebSocket>& rusty_websocket,
    SignalRMessageDecoder& decoder);

int main(int argc, char** argv)
try {
    if (argc != 4) {
        cout << "usage: " << argv[0]
             << " local_peer_id rusty_signal_server_host stun_server" << endl;
        exit(1);
    }
    string local_peer_id = argv[1];
    string rusty_signal_server_host = argv[2];
    string stun_server = argv[3];

    // Http post request
    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    string url_https =
        "https://" + local_peer_id + ":5001/sessionHub/negotiate?negotiateVersion=1";
    request.setOpt(new curlpp::options::Url(url_https));
    request.setOpt(new curlpp::options::Verbose(false));
    list<string> header;
    header.push_back("Content-Type: application/x-www-form-urlencoded");
    request.setOpt(new curlpp::options::HttpHeader(header));
    request.setOpt(new curlpp::options::SslVerifyPeer(false));
    std::ostringstream os;
    curlpp::options::WriteStream ws(&os);
    request.setOpt(ws);
    request.perform();
    os << request;
    HttpPostRequestReplyMessageDecoder decoder = HttpPostRequestReplyMessageDecoder();
    string error;
    if (!decoder.parseJSONMessage(os.str().c_str(), error)) {
        throw invalid_argument(error);
    }
    string https_token = decoder.getConnectionToken();

    SignalRMessageDecoder signalr_decoder = SignalRMessageDecoder();
    promise<void> wsr_promise;
    future<void> wsr_future = wsr_promise.get_future();
    rtc::Configuration signalr_config;
    signalr_config.iceServers.emplace_back("stun:" + stun_server);

    // SignalR websocket
    auto signalr_websocket = make_shared<rtc::WebSocket>();
    // RustySignal websocket
    auto rusty_websocket = make_shared<rtc::WebSocket>();

    bool client_id_checked = false;

    signalr_websocket->onOpen([&]() {
        cout << "SignalR WebSocket connected, signaling ready" << endl;
        wsr_promise.set_value();
    });

    signalr_websocket->onError([&](string const& error) {
        cout << "SignalR WebSocket failed: " << error << endl;
        wsr_promise.set_exception(make_exception_ptr(runtime_error(error)));
    });

    signalr_websocket->onClosed([&]() { cout << "SignalR WebSocket closed" << endl; });

    signalr_websocket->onMessage([&](variant<binary, string> data) {
        if (!holds_alternative<string>(data)) {
            return;
        }

        LOG_DEBUG_S << "signalr: " << get<string>(data) << std::endl;

        string error;
        if (!signalr_decoder.parseJSONMessage(get<string>(data).c_str(), error)) {
            throw invalid_argument(error);
        }

        // initial Handshake
        client_id_checked =
            initialHandShakeFinalized(signalr_websocket, signalr_decoder, local_peer_id);

        if (!client_id_checked) {
            return;
        }

        // workaround, since there is no unique action type field for everyone
        string actiontype;
        if (signalr_decoder.checkSdpMessage()) {
            actiontype = signalr_decoder.getActionType();
        }
        else if (signalr_decoder.checkCandidadeMessage()) {
            actiontype = "candidate";
        }
        else {
            return;
        }

        // Sdp negotiation parser
        if (rusty_websocket) {
            if (actiontype == "offer" || actiontype == "answer") {
                offerAnswerSignalMessageParser(rusty_websocket, signalr_decoder);
            }
            else if (actiontype == "candidate") {
                candidateSignalMessageParser(rusty_websocket, signalr_decoder);
            }
        }
    });

    // wss://localhost:5001/sessionHub?id=+connectionToken
    const string signalr_url_websocket =
        local_peer_id + ":5001/sessionHub?id=" + https_token;
    cout << "SignalR WebSocket URL is " << signalr_url_websocket << endl;
    signalr_websocket->open(signalr_url_websocket);
    sendInitialPayload(signalr_websocket);

    RustySignalMessageDecoder rusty_decoder = RustySignalMessageDecoder();
    promise<void> ws_promise;
    future<void> ws_future = ws_promise.get_future();
    rtc::Configuration rusty_config;
    rusty_config.iceServers.emplace_back("stun:" + stun_server);

    rusty_websocket->onOpen([&]() {
        cout << "RustySignal WebSocket connected, signaling ready" << endl;
        ws_promise.set_value();
    });

    rusty_websocket->onError([&](string const& error) {
        cout << "RustySignal WebSocket failed: " << error << endl;
        ws_promise.set_exception(make_exception_ptr(runtime_error(error)));
    });

    rusty_websocket->onClosed([&]() { cout << "RustySignal WebSocket closed" << endl; });

    rusty_websocket->onMessage([&](variant<binary, string> data) {
        if (!holds_alternative<string>(data)) {
            return;
        }

        LOG_DEBUG_S << "rusty: " << get<string>(data) << std::endl;

        string error;
        if (!rusty_decoder.parseJSONMessage(get<string>(data).c_str(), error)) {
            throw invalid_argument(error);
        }

        string actiontype = rusty_decoder.getActionType();

        if (actiontype == "ping" && client_id_checked) {
            pong(rusty_websocket, rusty_decoder, local_peer_id);
        }

        if (signalr_websocket) {
            if (actiontype == "offer" || actiontype == "answer") {
                offerAnswerRustyMessageParser(signalr_websocket, rusty_decoder);
            }
            else if (actiontype == "candidate") {
                candidateRustyMessageParser(signalr_websocket, rusty_decoder);
            }
        }
    });

    // wss://signalserverhost?user=yourname
    const string rusty_url_websocket =
        rusty_signal_server_host + "?user=" + local_peer_id;
    cout << "RustySignal WebSocket URL is " << rusty_url_websocket << endl;
    rusty_websocket->open(rusty_url_websocket);

    while (true) {
        string leave_session;
        cout << "Enter with \"leave_session\" to disconnect:" << endl;
        cin >> leave_session;
        cin.ignore();

        if (signalr_decoder.checkSessionClosed()) {
            cout << "Session closed: Vehicle server disconnected" << endl;
            break;
        }

        if (leave_session != "leave_session") {
            cout << "Invalid argument: \"leave_session\" to disconnect" << endl;
        }
        else {
            leaveSession(signalr_websocket, signalr_decoder, local_peer_id);
            break;
        }
    }

    return 0;
}
catch (exception const& error) {
    cout << "Error:" << error.what() << endl;
    return -1;
}

void rustySendJSON(shared_ptr<rtc::WebSocket>& websocket, Json::Value const& json)
{
    if (auto ws = make_weak_ptr(websocket).lock()) {
        Json::FastWriter fast;
        auto msg = fast.write(json);
        LOG_DEBUG_S << "rusty::sendJSON " << msg << std::endl;
        ws->send(msg);
    }
}

void signalRSendJSON(shared_ptr<rtc::WebSocket>& websocket, Json::Value const& json)
{
    if (auto ws = make_weak_ptr(websocket).lock()) {
        Json::FastWriter fast;
        auto msg = fast.write(json);
        LOG_DEBUG_S << "signalr::sendJSON " << msg << std::endl;
        ws->send(msg + "\x1e");
    }
}

void sendInitialPayload(shared_ptr<rtc::WebSocket>& websocket)
{
    Json::Value message;
    message["protocol"] = "json";
    message["version"] = 1;
    LOG_INFO_S << "signalr send initial payload (protocol/version)" << std::endl;
    signalRSendJSON(websocket, message);
}

void sendSessionCheck(shared_ptr<rtc::WebSocket>& signalr_websocket,
    string& local_peer_id)
{
    Json::Value message, content(Json::arrayValue), stream_ids(Json::arrayValue);
    // content.append("client_id:" + local_peer_id);
    Json::Value client_id;
    client_id["client_id"] = "foobar";
    content.append(client_id);
    message["arguments"] = content;
    message["invocationId"] = "0";
    stream_ids.append("blablabla");
    message["streamIds"] = stream_ids;
    message["target"] = "session_check";
    message["type"] = 1;
    LOG_INFO_S << "to   signalr session check" << std::endl;
    signalRSendJSON(signalr_websocket, message);
}

void joinSession(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id)
{
    Json::Value message, content(Json::arrayValue);
    content.append("client_id:" + local_peer_id);
    content.append("session_id:" + decoder.getSessionIdFromList());
    message["arguments"] = content;
    message["invocationId"] = "0";
    message["streamIds"] = Json::arrayValue;
    message["target"] = "join_session";
    message["type"] = 1;
    LOG_INFO_S << "to   signalr join session" << std::endl;
    signalRSendJSON(signalr_websocket, message);
}

void leaveSession(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id)
{
    Json::Value message, content(Json::arrayValue);
    content.append("client_id:" + local_peer_id);
    content.append("session_id:" + decoder.getSessionIdFromList());
    message["arguments"] = content;
    message["invocationId"] = "0";
    message["streamIds"] = Json::arrayValue;
    message["target"] = "leave_session";
    message["type"] = 1;
    LOG_INFO_S << "to   signalr leave session" << std::endl;
    signalRSendJSON(signalr_websocket, message);
}

void pong(shared_ptr<rtc::WebSocket>& rusty_websocket,
    RustySignalMessageDecoder& decoder,
    string const& local_peer_id)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getFrom();
    message["action"] = "pong";
    message["data"]["from"] = local_peer_id;
    LOG_INFO_S << "to   rusty   pong" << std::endl;
    rustySendJSON(rusty_websocket, message);
}

void offerAnswerRustyMessageParser(shared_ptr<rtc::WebSocket>& signalr_websocket,
    RustySignalMessageDecoder& decoder)
{
    Json::Value message;
    message["target"] = decoder.getTo();
    message["caller"] = decoder.getFrom();
    message["sdp_message"]["type"] = decoder.getActionType();
    message["sdp_message"]["sdp"] = decoder.getDescription();
    LOG_INFO_S << "to   signalr   send SDP offer" << std::endl;
    signalRSendJSON(signalr_websocket, message);
}

void candidateRustyMessageParser(shared_ptr<rtc::WebSocket>& signalr_websocket,
    RustySignalMessageDecoder& decoder)
{
    Json::Value message;
    message["target"] = decoder.getTo();
    message["candidate"] = decoder.getCandidate();
    LOG_INFO_S << "to   signalr   send candidate" << std::endl;
    signalRSendJSON(signalr_websocket, message);
}

void offerAnswerSignalMessageParser(shared_ptr<rtc::WebSocket>& rusty_websocket,
    SignalRMessageDecoder& decoder)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getTo();
    message["action"] = decoder.getActionType();
    message["data"]["from"] = decoder.getFrom();
    message["data"]["description"] = decoder.getDescription();
    LOG_INFO_S << "to   rusty   send offer" << std::endl;
    rustySendJSON(rusty_websocket, message);
}

void candidateSignalMessageParser(shared_ptr<rtc::WebSocket>& rusty_websocket,
    SignalRMessageDecoder& decoder)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getTo();
    message["action"] = decoder.getActionType();
    message["data"]["from"] = decoder.getFrom();
    message["data"]["candidate"] = decoder.getDescription();
    LOG_INFO_S << "to   rusty   send candidate" << std::endl;
    rustySendJSON(rusty_websocket, message);
}

bool initialHandShakeFinalized(shared_ptr<rtc::WebSocket>& signalr_websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id)
{
    if (decoder.isEmpty()) {
        sendSessionCheck(signalr_websocket, local_peer_id);
        return false;
    }
    if (decoder.checkSessionList()) {
        joinSession(signalr_websocket, decoder, local_peer_id);
        return false;
    }
    if (decoder.getClientId() == local_peer_id) {
        return true;
    }

    return false;
}
