#include "SignalRMessageDecoder.hpp"
#include <iostream>

using namespace std;
using namespace deep_trekker;

SignalRMessageDecoder::SignalRMessageDecoder()
    : mReader(mRBuilder.newCharReader())
{
}

bool SignalRMessageDecoder::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void SignalRMessageDecoder::validateFieldPresent(
    Json::Value const& value,
    string const& fieldName
)
{
    if (!value.isMember(fieldName))
    {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

bool SignalRMessageDecoder::isEmpty()
{
    return mJData.empty();
}

bool SignalRMessageDecoder::checkSdpMessage()
{
    return mJData.isMember("sdp_message");
}

bool SignalRMessageDecoder::checkCandidadeMessage()
{
    return mJData.isMember("candidate");
}

bool SignalRMessageDecoder::checkSessionList()
{
     return mJData[0].isMember("session_id");
}

string SignalRMessageDecoder::getClientId()
{
    validateFieldPresent(mJData, "session_id");
    return mJData["session_id"].asString();
}

string SignalRMessageDecoder::getSessionIdFromList()
{
    validateFieldPresent(mJData[0], "session_id");
    return mJData[0]["session_id"].asString();
}

string SignalRMessageDecoder::getActionType()
{
    validateFieldPresent(mJData, "sdp_message");
    validateFieldPresent(mJData, "type");
    return mJData["type"].asString();
}

string SignalRMessageDecoder::getTo()
{
    validateFieldPresent(mJData, "target");
    return mJData["target"].asString();
}

string SignalRMessageDecoder::getFrom()
{
    validateFieldPresent(mJData, "caller");
    return mJData["caller"].asString();
}

string SignalRMessageDecoder::getDescription()
{
    validateFieldPresent(mJData, "sdp_message");
    validateFieldPresent(mJData, "type");
    return mJData["sdp"].asString();
}

string SignalRMessageDecoder::getCandidate()
{
    validateFieldPresent(mJData, "candidate");
    return mJData["candidate"].asString();
}
