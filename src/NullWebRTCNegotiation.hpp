#ifndef DEEP_TREKKER_NULLWEBRTCNEGOTIATION_HPP
#define DEEP_TREKKER_NULLWEBRTCNEGOTIATION_HPP

#include <deep_trekker/WebRTCNegotiationInterface.hpp>

namespace deep_trekker {
    class NullWebRTCNegotiation : public WebRTCNegotiationInterface {
    public:
        virtual ~NullWebRTCNegotiation();
        /** Publish a offer/answer */
        void publishDescription(std::string const& type, std::string const& sdp) override;
        /** Publish a candidate */
        void publishICECandidate(std::string const& candidate) override;
    };
}

#endif