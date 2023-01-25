rock_library(deep_trekker
    SOURCES CommandAndStateMessageParser.cpp
            RustySignalMessageDecoder.cpp
            SignalRMessageDecoder.cpp
            HttpPostRequestReplyMessageDecoder.cpp
    HEADERS CommandAndStateMessageParser.hpp
            DeepTrekkerCommands.hpp
            DeepTrekkerStates.hpp
            RustySignalMessageDecoder.hpp
            SignalRMessageDecoder.hpp
            HttpPostRequestReplyMessageDecoder.hpp
    DEPS_PKGCONFIG base-types power_base jsoncpp base-logging)


rock_executable(signaling_bridge Main.cpp
    DEPS deep_trekker
    DEPS_PKGCONFIG libdatachannel curlpp)

# Needed by libdatachannel
set_source_files_properties( Main.cpp PROPERTIES COMPILE_FLAGS "-std=c++17" )
