#ifndef DEEP_TREKKER_SIGNALR_HPP
#define DEEP_TREKKER_SIGNALR_HPP

#include <memory>
#include <queue>

#include <json/json.h>
#include <rtc/rtc.hpp>

#include <deep_trekker/NullWebRTCNegotiation.hpp>
#include <deep_trekker/SynchronousWebSocket.hpp>
#include <deep_trekker/WebRTCNegotiationInterface.hpp>

namespace deep_trekker {
    /** Interface to Deep Trekker's SignalR-based signalling
     */
    class SignalR : public WebRTCNegotiationInterface {
    public:
        enum States {
            STATE_PENDING,
            STATE_HANDSHAKE,
            STATE_SESSION_CHECK,
            STATE_SESSION_JOIN,
            STATE_READY,
            STATE_SESSION_LEAVE,

            STATE_FATAL_ERRORS,
            STATE_CONNECTION_LOST,
            STATE_PROTOCOL_ERROR,
            STATE_JSON_ERROR
        };

    private:
        SynchronousWebSocket m_ws;
        std::string m_host;
        std::string m_rock_peer_id;
        std::string m_deep_trekker_peer_id;
        base::Time m_timeout;

        std::string m_token;

        std::mutex m_state_lock;
        std::condition_variable m_state_wait;
        States m_state = STATE_PENDING;
        void setState(States new_state);

        template <typename Lock>
        void waitState(States state, Lock& lock, base::Time const& timeout);

        int m_last_used_invocation_id = 0;
        int m_last_received_invocation_id = 0;
        std::string m_session_id;
        std::queue<Json::Value> m_message_queue;
        Json::Value m_signalr_context;

        std::string getNextInvocationID();

        void call(std::string const& target, Json::Value const& arg);
        void invoke(Json::Value message);
        bool hasReceivedReply() const;
        Json::Value processReply(Json::Value const& ret);

        WebRTCNegotiationInterface* m_listener = new NullWebRTCNegotiation();

        void process(Json::Value const& msg);

        void handshake();
        void sessionCheck();
        void sessionJoin();
        void sessionLeave();

        /** Synchronous initial negotiation phase
         *
         * This must be called first, since it gathers the connection token needed
         * to open the websocket
         */
        void negotiate(bool curl_verbose = false);

        /** Synchronously open the websocket, and asynchronously start the
         * session protocol
         *
         * It does wait for the websocket ot be opened, but does not wait for
         * the session join to finish
         */
        void open();

    public:
        /** Connects to the deep-trekker-provided SignalR server and start negotiating
         *
         * The constructor returns when the websocket is opened and the first handshake
         * message returned successfully
         */
        SignalR(rtc::WebSocket::Configuration const& config,
            std::string const& host,
            std::string const& rock_peer_id,
            std::string const& deep_trekker_peer_id,
            bool curl_verbose = false,
            base::Time const& timeout = base::Time::fromSeconds(2));
        virtual ~SignalR();

        void start();
        void setListener(WebRTCNegotiationInterface* listener);

        void waitState(States state,
            base::Time const& timeout = base::Time::fromSeconds(1));
        void waitState(States state,
            std::function<void()> f,
            base::Time const& timeout = base::Time::fromSeconds(1));

        void publishICECandidate(std::string const& candidate) override;
        void publishDescription(std::string const& type, std::string const& sdp) override;

        void ping() override;
        void pong() override;
    };
}

#endif