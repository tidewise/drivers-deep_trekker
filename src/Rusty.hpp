#ifndef DEEP_TREKKER_RUSTY_HPP
#define DEEP_TREKKER_RUSTY_HPP

#include <base/Timeout.hpp>
#include <deep_trekker/NullWebRTCNegotiation.hpp>
#include <deep_trekker/SynchronousWebSocket.hpp>
#include <deep_trekker/WebRTCNegotiationInterface.hpp>
#include <rtc/rtc.hpp>

namespace deep_trekker {
    /** Interface to the rusty side of the signalling
     */
    class Rusty : public WebRTCNegotiationInterface {
        SynchronousWebSocket m_ws;

        std::string m_host;
        std::string m_rock_peer_id;
        std::string m_deep_trekker_peer_id;
        base::Time m_timeout;

        bool m_has_new_client = false;
        std::weak_ptr<WebRTCNegotiationInterface> m_client;
        std::mutex m_on_new_client_lock;
        std::condition_variable m_on_new_client;

        WebRTCNegotiationInterface* m_listener = new NullWebRTCNegotiation();

        void open();
        void sendPingPong(std::string const& type);

    public:
        Rusty(rtc::WebSocket::Configuration const& config,
            std::string const& host,
            std::string const& rusty_peer_id,
            std::string const& deep_trekker_peer_id,
            base::Time const& timeout = base::Time::fromSeconds(2));
        ~Rusty();

        void waitNewClient();
        bool setClient(std::shared_ptr<WebRTCNegotiationInterface> client);

        void publishICECandidate(std::string const& candidate) override;
        void publishDescription(std::string const& type, std::string const& sdp) override;
        void ping() override;
        void pong() override;
    };
}

#endif