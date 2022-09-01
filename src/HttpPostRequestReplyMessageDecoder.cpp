#include "HttpPostRequestReplyMessageDecoder.hpp"
#include <iostream>

using namespace std;
using namespace deep_trekker;

HttpPostRequestReplyMessageDecoder::HttpPostRequestReplyMessageDecoder() : mReader(mRBuilder.newCharReader()) {}

bool HttpPostRequestReplyMessageDecoder::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void HttpPostRequestReplyMessageDecoder::validateFieldPresent(
    Json::Value const& value,
    string const& fieldName
)
{
    if (!value.isMember(fieldName))
    {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

string HttpPostRequestReplyMessageDecoder::getConnectionToken()
{
    validateFieldPresent(mJData, "connectionToken");
    return mJData["connectionToken"].asString();
}

