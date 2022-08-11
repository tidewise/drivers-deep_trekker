#include <future>
#include <sstream>
#include <iostream>
#include "rtc/rtc.hpp"
#include "json/json.h"
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <deep_trekker/HttpPostRequestReplyMessageDecoder.hpp>
#include <deep_trekker/RustySignalMessageDecoder.hpp>
#include <deep_trekker/SignalRMessageDecoder.hpp>

using namespace deep_trekker;
using namespace std;
using namespace rtc;
using std::shared_ptr;
using std::weak_ptr;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }
void pong(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder,
    string const& local_peer_id
);
void sendInitialPayload(shared_ptr<rtc::WebSocket>& websocket);
void sendSessionCheck(shared_ptr<rtc::WebSocket>& websocket, string& local_peer_id);
void joinSession(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id
);
void offerAnswerRustyMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder
);
void candidateRustyMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder
);
void offerAnswerSignalMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder
);
void candidateSignalMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder
);

int main(int argc, char** argv)
try
{
    if (argc != 2)
    {
        cout << "usage: " << argv[0] << " local peer id" << endl;
        exit(1);
    }
    string local_peer_id = argv[1];

    // Http post request
    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    string url_https =
        "https://" + local_peer_id + ":5001/sessionHub/negotiate?negotiateVersion=1";
    request.setOpt(new curlpp::options::Url(url_https));
    request.setOpt(new curlpp::options::Verbose(true));
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
    if (!decoder.parseJSONMessage(os.str().c_str(), error))
    {
        throw invalid_argument(error);
    }
    string https_token = decoder.getConnectionToken();

    SignalRMessageDecoder signalr_decoder = SignalRMessageDecoder();
    promise<void> wsr_promise;
    future<void> wsr_future = wsr_promise.get_future();
    rtc::Configuration signalr_config;
    // TODO - make it configurable
    signalr_config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    // SignalR websocket
    auto signalr_websocket = make_shared<rtc::WebSocket>();
    // RustySignal websocket
    auto rusty_websocket = make_shared<rtc::WebSocket>();

    bool empty_json_received = false;
    bool session_list_checked = false;
    bool client_id_checked = false;

    signalr_websocket->onOpen(
        [&]()
        {
            cout << "SignalR WebSocket connected, signaling ready" << endl;
            wsr_promise.set_value();
        }
    );

    signalr_websocket->onError(
        [&](string const& error)
        {
            cout << "SignalR WebSocket failed: " << error << endl;
            wsr_promise.set_exception(make_exception_ptr(runtime_error(error)));
        }
    );

    signalr_websocket->onClosed([&]() { cout << "SignalR WebSocket closed" << endl; });

    signalr_websocket->onMessage(
        [&](variant<binary, string> data)
        {
            if (!holds_alternative<string>(data))
            {
                return;
            }

            string error;
            if (!signalr_decoder.parseJSONMessage(get<string>(data).c_str(), error))
            {
                throw invalid_argument(error);
            }

            if (signalr_decoder.isEmpty() && !empty_json_received)
            {
                empty_json_received = true;
                sendSessionCheck(signalr_websocket, local_peer_id);
                return;
            }

            if (signalr_decoder.checkSessionList() && !session_list_checked)
            {
                session_list_checked = true;
                joinSession(signalr_websocket, signalr_decoder, local_peer_id);
                return;
            }

            if (signalr_decoder.getClientId() == local_peer_id && !client_id_checked)
            {
                client_id_checked = true;
                return;
            }

            // workaround, since there is no unique action type field for everyone
            string actiontype;
            if (signalr_decoder.checkSdpMessage())
            {
                actiontype = signalr_decoder.getActionType();
            }
            else if (signalr_decoder.checkCandidadeMessage())
            {
                actiontype = "candidate";
            }
            else
            {
                return;
            }
            
            if (rusty_websocket)
            {
                if (actiontype == "offer" || actiontype == "answer")
                {
                    offerAnswerSignalMessageParser(rusty_websocket, signalr_decoder);
                }
                else if (actiontype == "candidate")
                {
                    candidateSignalMessageParser(rusty_websocket, signalr_decoder);
                }
            }
        }
    );

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
    // TODO - make it configurable
    rusty_config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    rusty_websocket->onOpen(
        [&]()
        {
            cout << "RustySignal WebSocket connected, signaling ready" << endl;
            ws_promise.set_value();
        }
    );

    rusty_websocket->onError(
        [&](string const& error)
        {
            cout << "RustySignal WebSocket failed: " << error << endl;
            ws_promise.set_exception(make_exception_ptr(runtime_error(error)));
        }
    );

    rusty_websocket->onClosed([&]() { cout << "RustySignal WebSocket closed" << endl; });

    rusty_websocket->onMessage(
        [&](variant<binary, string> data)
        {
            if (!holds_alternative<string>(data))
            {
                return;
            }

            string error;
            if (!rusty_decoder.parseJSONMessage(get<string>(data).c_str(), error))
            {
                throw invalid_argument(error);
            }

            string actiontype = rusty_decoder.getActionType();

            if (actiontype == "ping" && client_id_checked)
            {
                pong(rusty_websocket, rusty_decoder, local_peer_id);
            }

            if (signalr_websocket)
            {
                if (actiontype == "offer" || actiontype == "answer")
                {
                    offerAnswerRustyMessageParser(signalr_websocket, rusty_decoder);
                }
                else if (actiontype == "candidate")
                {
                    candidateRustyMessageParser(signalr_websocket, rusty_decoder);
                }
            }
        }
    );

    // wss://signalserverhost?user=yourname
    const string rusty_url_websocket = "127.0.0.1:3012?user=" + local_peer_id;
    cout << "RustySignal WebSocket URL is " << rusty_url_websocket << endl;
    rusty_websocket->open(rusty_url_websocket);

    return 0;
}
catch (exception const& error)
{
    cout << "Error:" << error.what() << endl;
}

void sendInitialPayload(shared_ptr<rtc::WebSocket>& websocket)
{
    Json::Value message;
    message["protocol"] = "json";
    message["version"] = "1";
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void sendSessionCheck(shared_ptr<rtc::WebSocket>& websocket, string& local_peer_id)
{
    Json::Value message, content(Json::arrayValue);
    content.append(local_peer_id);
    message["arguments"] = content;
    message["invocationId"] = "0";
    message["streamIds"] = Json::arrayValue;
    message["target"] = "session_check";
    message["type"] = 1;
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void joinSession(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder,
    string& local_peer_id
)
{
    Json::Value message, content(Json::arrayValue);
    content.append(local_peer_id);
    content.append(decoder.getSessionIdFromList());
    message["arguments"] = content;
    message["invocationId"] = "0";
    message["streamIds"] = Json::arrayValue;
    message["target"] = "join_session";
    message["type"] = 1;
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void pong(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder,
    string const& local_peer_id
)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getFrom();
    message["action"] = "pong";
    message["data"]["from"] = local_peer_id;
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void offerAnswerRustyMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder
)
{
    Json::Value message;
    message["target"] = decoder.getTo();
    message["caller"] = decoder.getFrom();
    message["sdp_message"]["type"] = decoder.getActionType();
    message["sdp_message"]["sdp"] = decoder.getDescription();
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void candidateRustyMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    RustySignalMessageDecoder& decoder
)
{
    Json::Value message;
    message["target"] = decoder.getTo();
    message["candidate"] = decoder.getCandidate();
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void offerAnswerSignalMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder
)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getTo();
    message["action"] = decoder.getActionType();
    message["data"]["from"] = decoder.getFrom();
    message["data"]["description"] = decoder.getDescription();

    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}

void candidateSignalMessageParser(
    shared_ptr<rtc::WebSocket>& websocket,
    SignalRMessageDecoder& decoder
)
{
    Json::Value message;
    message["protocol"] = "one-to-one";
    message["to"] = decoder.getTo();
    message["action"] = decoder.getActionType();
    message["data"]["from"] = decoder.getFrom();
    message["data"]["candidate"] = decoder.getDescription();
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        Json::FastWriter fast;
        ws->send(fast.write(message));
    }
}
