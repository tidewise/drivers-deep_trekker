#ifndef DEEP_TREKKER_SYNCHRONOUSWEBSOCKET_HPP
#define DEEP_TREKKER_SYNCHRONOUSWEBSOCKET_HPP

#include <base/Time.hpp>
#include <functional>
#include <json/json.h>
#include <rtc/rtc.hpp>

namespace deep_trekker
{
    /** A thin wrapper over rtc::Websocket providing synchronous operations
     */
    class SynchronousWebSocket
    {
    public:
        typedef std::function<void(std::string const &)> OnError;
        typedef std::function<void(Json::Value const &)> OnJSONMessage;

    private:
        rtc::WebSocket m_ws;
        std::string m_debug_name;

        Json::CharReader *m_json_reader = nullptr;
        OnError m_on_error;
        OnError m_on_json_error;
        OnJSONMessage m_on_json_message;

        void dispatchMessage(std::string const &msg);

    public:
        SynchronousWebSocket(std::string const &debug_name = "");
        SynchronousWebSocket(rtc::WebSocket::Configuration const &config,
                             std::string const &debug_name = "");
        ~SynchronousWebSocket();

        /** Synchronously open the websocket
         *
         * At the end of the call, either the websocket is opened, or the
         * method throws an exception after `timeout` elapsed
         */
        void open(std::string const &url, base::Time const &timeout);

        /** Synchronously close the websocket
         *
         * At the end of the call, either the websocket is closed, or the
         * method throws an exception after `timeout` elapsed
         */
        void close(base::Time const &timeout);

        /** Send a message */
        void send(Json::Value const &msg);

        /** Send a message */
        void send(std::string const &msg);

        /** Register a callback to receive messages parsed as JSON */
        void onJSONMessage(OnJSONMessage callback);
        /** Register a callback to receive errors during JSON parsing */
        void onJSONError(OnError callback);
        /** Register a callback to receive websocket errors */
        void onWebSocketError(OnError callback);

        Json::Value jsonParse(std::string const &msg);
        static std::string jsonToString(Json::Value const &arg);
    };
}

#endif