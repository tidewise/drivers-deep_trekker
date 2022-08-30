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
    DEPS_PKGCONFIG base-types power_base jsoncpp)


rock_executable(deep_trekker_bin Main.cpp
    DEPS deep_trekker
    DEPS_PKGCONFIG libdatachannel curlpp)
