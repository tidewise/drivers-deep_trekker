#include <deep_trekker/NullWebRTCNegotiation.hpp>

using namespace deep_trekker;

NullWebRTCNegotiation::~NullWebRTCNegotiation()
{
}
void NullWebRTCNegotiation::publishDescription(std::string const& type,
    std::string const& sdp)
{
}
void NullWebRTCNegotiation::publishICECandidate(std::string const& candidate)
{
}