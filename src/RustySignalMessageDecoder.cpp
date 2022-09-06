#include "RustySignalMessageDecoder.hpp"
#include <iostream>

using namespace std;
using namespace deep_trekker;

RustySignalMessageDecoder::RustySignalMessageDecoder()
    : mReader(mRBuilder.newCharReader())
{
}

bool RustySignalMessageDecoder::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void RustySignalMessageDecoder::validateFieldPresent(
    Json::Value const& value,
    string const& fieldName
)
{
    if (!value.isMember(fieldName))
    {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

string RustySignalMessageDecoder::getActionType()
{
    validateFieldPresent(mJData, "action");
    return mJData["action"].asString();
}

string RustySignalMessageDecoder::getTo()
{
    validateFieldPresent(mJData, "to");
    return mJData["to"].asString();
}

string RustySignalMessageDecoder::getFrom()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "from");
    return mJData["data"]["from"].asString();
}

string RustySignalMessageDecoder::getDescription()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "description");
    return mJData["data"]["description"].asString();
}

string RustySignalMessageDecoder::getCandidate()
{
    validateFieldPresent(mJData, "data");
    validateFieldPresent(mJData["data"], "candidate");
    return mJData["data"]["candidate"].asString();
}
