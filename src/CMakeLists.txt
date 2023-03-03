rock_library(deep_trekker
    SOURCES CommandAndStateMessageParser.cpp
            NullWebRTCNegotiation.cpp
            Rusty.cpp
            SynchronousWebSocket.cpp
    HEADERS CommandAndStateMessageParser.hpp
            DeepTrekkerCommands.hpp
            DeepTrekkerStates.hpp
            WebRTCNegotiationInterface.hpp
            NullWebRTCNegotiation.hpp
            Rusty.hpp
            SynchronousWebSocket.hpp
    DEPS_PKGCONFIG base-types power_base jsoncpp base-logging libdatachannel)

rock_library(signalr
    SOURCES SignalR.cpp
    HEADERS SignalR.hpp
    DEPS_PKGCONFIG curlpp)

target_link_libraries(signalr deep_trekker)

find_package(Threads REQUIRED)
rock_executable(deep_trekker_signaling_bridge Main.cpp DEPS signalr)
target_link_libraries(deep_trekker_signaling_bridge Threads::Threads)
