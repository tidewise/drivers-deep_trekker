rock_library(deep_trekker
    SOURCES CommandAndStateMessageParser.cpp
            NullWebRTCNegotiation.cpp
            SignalR.cpp
            Rusty.cpp
            SynchronousWebSocket.cpp
    HEADERS CommandAndStateMessageParser.hpp
            DeepTrekkerCommands.hpp
            DeepTrekkerStates.hpp
            WebRTCNegotiationInterface.hpp
            NullWebRTCNegotiation.hpp
            SignalR.hpp
            Rusty.hpp
            SynchronousWebSocket.hpp
    DEPS_PKGCONFIG base-types power_base jsoncpp base-logging libdatachannel curlpp)

find_package(Threads REQUIRED)
rock_executable(deep_trekker_signaling_bridge Main.cpp
    DEPS deep_trekker)
target_link_libraries(deep_trekker_signaling_bridge Threads::Threads)