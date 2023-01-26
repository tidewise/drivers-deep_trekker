#include <base-logging/Logging.hpp>
#include <deep_trekker/SynchronousWebSocket.hpp>

using namespace deep_trekker;
using namespace rtc;
using namespace std;

SynchronousWebSocket::SynchronousWebSocket(string const& debug_name)
    : SynchronousWebSocket(WebSocket::Configuration(), debug_name)
{
}

SynchronousWebSocket::~SynchronousWebSocket()
{
    delete m_json_reader;
}

SynchronousWebSocket::SynchronousWebSocket(WebSocket::Configuration const& config,
    string const& debug_name)
    : m_ws(config)
    , m_debug_name(debug_name)
{
    Json::CharReaderBuilder builder;
    m_json_reader = builder.newCharReader();

    m_ws.onMessage([&](std::variant<rtc::binary, string> const& data) {
        if (!holds_alternative<string>(data)) {
            m_on_json_error("received binary message, expected string");
            return;
        }

        auto whole_msg = get<string>(data);
        string msg;
        // \x1e is the separator in SignalR
        stringstream whole_msg_ss(whole_msg);
        while (getline(whole_msg_ss, msg, '\x1e')) {
            dispatchMessage(msg);
        }
    });
}

void SynchronousWebSocket::dispatchMessage(string const& msg)
{
    Json::Value json;
    try {
        LOG_DEBUG_S << "< " << m_debug_name << ": " << msg << endl;
        json = jsonParse(msg);
    }
    catch (std::exception& e) {
        m_on_json_error(e.what());
    }

    try {
        m_on_json_message(json);
    }
    catch (std::exception& e) {
        LOG_ERROR_S << m_debug_name << ": unhandled exception in JSON message handler";
        LOG_ERROR_S << m_debug_name << ": " << e.what();
    }
}

void SynchronousWebSocket::open(string const& url, base::Time const& timeout)
{
    promise<void> promise;
    auto future = promise.get_future();

    m_ws.onOpen([&promise, name = m_debug_name]() {
        LOG_DEBUG_S << "websocket opened to " << name;
        promise.set_value();
    });
    m_ws.onError([&promise, name = m_debug_name](string const& error) {
        LOG_DEBUG_S << "websocket error to " << name;
        promise.set_exception(make_exception_ptr(runtime_error(error)));
    });

    m_ws.open(url);
    if (future.wait_for(chrono::microseconds(timeout.toMicroseconds())) !=
        future_status::ready) {
        throw std::runtime_error(
            "timed out waiting for the websocket connection with " + m_debug_name);
    }
    future.get();

    m_ws.onError(m_on_error);

    LOG_DEBUG_S << "successfully opened connection to " << m_debug_name;
}

void SynchronousWebSocket::onJSONMessage(OnJSONMessage callback)
{
    m_on_json_message = callback;
}

void SynchronousWebSocket::onJSONError(OnError callback)
{
    m_on_json_error = callback;
}

void SynchronousWebSocket::onWebSocketError(OnError callback)
{
    m_on_error = callback;
    if (m_ws.readyState() == WebSocket::State::Open) {
        m_ws.onError(m_on_error);
    }
}

void SynchronousWebSocket::send(Json::Value const& msg)
{
    send(jsonToString(msg));
}

void SynchronousWebSocket::send(std::string const& msg)
{
    LOG_DEBUG_S << "> " << m_debug_name << ": " << msg << endl;
    m_ws.send(msg);
}

void SynchronousWebSocket::close(base::Time const& timeout)
{
    promise<void> promise;
    auto future = promise.get_future();

    m_ws.onClosed([&promise]() { promise.set_value(); });
    m_ws.onError([&promise](string const& error) {
        promise.set_exception(make_exception_ptr(runtime_error(error)));
    });

    m_ws.close();
    if (future.wait_for(chrono::microseconds(timeout.toMicroseconds())) !=
        future_status::ready) {
        throw std::runtime_error("timed out waiting for the websocket connection");
    }
    future.get();
}

Json::Value SynchronousWebSocket::jsonParse(std::string const& msg)
{
    char const* begin = msg.data();
    char const* end = begin + msg.size();
    string error;

    Json::Value json;
    if (!m_json_reader->parse(begin, end, &json, &error)) {
        throw std::runtime_error(error);
    }

    return json;
}

string SynchronousWebSocket::jsonToString(Json::Value const& json)
{
    Json::FastWriter fast;
    return fast.write(json);
}
