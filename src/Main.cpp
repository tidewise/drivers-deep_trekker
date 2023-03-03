#include <base-logging/Logging.hpp>
#include <deep_trekker/Rusty.hpp>
#include <deep_trekker/SignalR.hpp>
#include <iostream>
#include <sstream>

using namespace deep_trekker;
using namespace std;
using namespace rtc;
using std::shared_ptr;

rtcLogLevel rtcLogLevelFromString(string const& str);

shared_ptr<SignalR> createSignalR(shared_ptr<Rusty> rusty,
    string const& signalr_host,
    string const& rock_peer_id,
    string const& deep_trekker_peer_id);

int main(int argc, char** argv)
{
    if (argc != 5) {
        cout << "usage: " << argv[0]
             << " rock_id deep_trekker_id signalr_host_port rusty_host_port"
             << "       " << argv[0]
             << " rock revolution 192.168.88.53:5001 localhost:3012" << endl;
        exit(1);
    }
    string rock_peer_id = argv[1];
    string deep_trekker_peer_id = argv[2];
    string signalr_host = argv[3];
    string rusty_host = argv[4];

    auto env_c = getenv("RTC_LOG_LEVEL");
    if (env_c) {
        string env(env_c);
        rtcInitLogger(rtcLogLevelFromString(env), nullptr);
    }

    rtc::WebSocket::Configuration rusty_config;

    while (true) {
        std::shared_ptr<Rusty> rusty = make_shared<Rusty>(rusty_config,
            rusty_host,
            rock_peer_id,
            deep_trekker_peer_id);

        rusty->waitClientNew();

        LOG_INFO_S << "Opening connection to Deep Trekker";
        std::shared_ptr<SignalR> signalr =
            createSignalR(rusty, signalr_host, rock_peer_id, deep_trekker_peer_id);
        signalr->setListener(rusty);
        if (rusty->setClient(signalr)) {
            LOG_INFO_S << "Starting negotiation";
            signalr->start();
            signalr->waitState(SignalR::STATE_READY);
        }

        rusty->waitClientEnd();
        rusty.reset();
        signalr.reset();
        LOG_INFO_S << "Rusty client end, waiting for new client";
    }
}

rtcLogLevel rtcLogLevelFromString(string const& str)
{
    if (str == "DEBUG") {
        return RTC_LOG_VERBOSE;
    }
    if (str == "INFO") {
        return RTC_LOG_INFO;
    }
    if (str == "WARN") {
        return RTC_LOG_WARNING;
    }
    if (str == "ERROR") {
        return RTC_LOG_ERROR;
    }
    if (str == "FATAL") {
        return RTC_LOG_FATAL;
    }
    else {
        std::cerr << "BASE_LOG_LEVEL=" << str
                  << " not recognized, setting libdatachannel log level to Warning"
                  << std::endl;
        return RTC_LOG_WARNING;
    }
}

std::shared_ptr<SignalR> createSignalR(shared_ptr<Rusty> rusty,
    string const& signalr_host,
    string const& rock_peer_id,
    string const& deep_trekker_peer_id)
{
    rtc::WebSocket::Configuration signalr_config;
    signalr_config.disableTlsVerification = true;
    unique_ptr<SignalR> signalr(
        new SignalR(signalr_config, signalr_host, rock_peer_id, deep_trekker_peer_id));
    signalr->setListener(rusty);
    return signalr;
}