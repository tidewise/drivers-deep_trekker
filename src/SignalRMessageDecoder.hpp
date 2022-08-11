#ifndef _SIGNALR_MESSAGE_DECODER_HPP_
#define _SIGNALR_MESSAGE_DECODER_HPP_

#include "stdio.h"
#include "string.h"
#include "json/json.h"
#include <memory>

namespace deep_trekker
{

    class SignalRMessageDecoder
    {
      public:
        SignalRMessageDecoder();

        std::string getActionType();
        std::string getTo();
        std::string getFrom();
        std::string getDescription();
        std::string getCandidate();
        std::string getSessionIdFromList();
        std::string getClientId();
        bool checkSdpMessage();
        bool checkCandidadeMessage();
        bool isEmpty();
        bool checkSessionList();
        bool parseJSONMessage(char const* data, std::string& errors);
        void validateFieldPresent(Json::Value const& value, std::string const& fieldName);

      private:
        Json::Value mJData;
        Json::CharReaderBuilder mRBuilder;
        std::unique_ptr<Json::CharReader> const mReader;
    };

} // namespace deep_trekker

#endif
