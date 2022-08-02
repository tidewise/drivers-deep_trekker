
#include "rtc/rtc.hpp"
#include "json/json.h"
#include <curlpp/cURLpp.hpp>
#include <deep_trekker/RustySignalMessageDecoder.hpp>
#include <future>
#include <iostream>

using namespace deep_trekker;
using namespace std;
using namespace rtc;
using std::shared_ptr;
using std::weak_ptr;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }
void pong(shared_ptr<rtc::WebSocket>& websocket,
          RustySignalMessageDecoder& decoder,
          string const& local_peer_id);

shared_ptr<rtc::PeerConnection> peerConnection;

int main(int argc, char** argv)
try
{
    if (argc != 2)
    {
        cout << "usage: " << argv[0] << " local peer id" << endl;
        exit(1);
    }
    string local_peer_id = argv[1];

    RustySignalMessageDecoder rusty_decoder = RustySignalMessageDecoder();

    promise<void> ws_promise;
    future<void> ws_future = ws_promise.get_future();
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");
    auto rusty_websocket = std::make_shared<rtc::WebSocket>();

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

            if (actiontype == "ping")
            {
                pong(rusty_websocket, rusty_decoder, local_peer_id);
            }
        }
    );

    // wss://signalserverhost?user=yourname
    const string url = "127.0.0.1:3012?user=" + local_peer_id;
    std::cout << "RustySignal WebSocket URL is " << url << std::endl;
    rusty_websocket->open(url);

    return 0;
}
catch (exception const& error)
{
    cout << "Error:" << error.what() << endl;
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
    Json::FastWriter fast;
    if (auto ws = make_weak_ptr(websocket).lock())
    {
        ws->send(fast.write(message));
    }
}
