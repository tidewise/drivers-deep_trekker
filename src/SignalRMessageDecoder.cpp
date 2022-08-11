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
    validateFieldPresent(mJData, "action");
    return mJData["action"].asString();
}

string SignalRMessageDecoder::getTo()
{
    validateFieldPresent(mJData, "to");
    return mJData["to"].asString();
}

string SignalRMessageDecoder::getFrom()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "from");
    return mJData["data"]["from"].asString();
}

string SignalRMessageDecoder::getDescription()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "description");
    return mJData["data"]["description"].asString();
}

string SignalRMessageDecoder::getCandidate()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "candidate");
    return mJData["data"]["candidate"].asString();
}

string SignalRMessageDecoder::getMid()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "mid");
    return mJData["data"]["mid"].asString();
}
