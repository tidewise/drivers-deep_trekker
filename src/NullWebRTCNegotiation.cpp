#include <deep_trekker/NullWebRTCNegotiation.hpp>
#include <memory>

using namespace deep_trekker;
using namespace std;

NullWebRTCNegotiation::~NullWebRTCNegotiation()
{
}
void NullWebRTCNegotiation::publishDescription(std::string const& type,
    std::string const& sdp)
{
}
void NullWebRTCNegotiation::publishICECandidate(std::string const& candidate,
    std::string const& mid)
{
}
void NullWebRTCNegotiation::ping()
{
}
void NullWebRTCNegotiation::pong()
{
}

static shared_ptr<NullWebRTCNegotiation> instance = make_shared<NullWebRTCNegotiation>();
shared_ptr<NullWebRTCNegotiation> NullWebRTCNegotiation::instance()
{
    return ::instance;
}