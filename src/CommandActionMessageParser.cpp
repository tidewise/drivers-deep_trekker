#include "CommandActionMessageParser.hpp"
#include <iostream>

using namespace std;
using namespace deep_trekker;

CommandActionMessageParser::CommandActionMessageParser() : mReader(mRBuilder.newCharReader()) {}

bool CommandActionMessageParser::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void CommandActionMessageParser::validateFieldPresent(
    Json::Value const& value,
    string const& fieldName
)
{
    if (!value.isMember(fieldName))
    {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

string CommandActionMessageParser::parseGetMessage()
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "GET";
    message["payload"] = {};

    return message.asString();
}
