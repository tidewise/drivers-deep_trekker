#ifndef _HTTP_POST_REQUEST_REPLY_MESSAGE_DECODER_HPP_
#define _HTTP_POST_REQUEST_REPLY_MESSAGE_DECODER_HPP_

#include "stdio.h"
#include "string.h"
#include "json/json.h"
#include <memory>

namespace deep_trekker
{

    class HttpPostRequestReplyMessageDecoder
    {
      public:
        HttpPostRequestReplyMessageDecoder();

        std::string getConnectionToken();
        bool parseJSONMessage(char const* data, std::string& errors);
        void validateFieldPresent(Json::Value const& value, std::string const& fieldName);

      private:
        Json::Value mJData;
        Json::CharReaderBuilder mRBuilder;
        std::unique_ptr<Json::CharReader> const mReader;
    };

} // namespace deep_trekker

#endif
