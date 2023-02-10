#ifndef DEEP_TREKKER_WEBRTCNEGOTIATIONINTERFACE_HPP
#define DEEP_TREKKER_WEBRTCNEGOTIATIONINTERFACE_HPP

#include <string>

namespace deep_trekker {
    struct WebRTCNegotiationInterface {
        virtual ~WebRTCNegotiationInterface()
        {
        }
        virtual void publishDescription(std::string const& type,
            std::string const& sdp) = 0;
        virtual void publishICECandidate(std::string const& candidate,
            std::string const& mid) = 0;
        virtual void ping() = 0;
        virtual void pong() = 0;
    };
}

#endif