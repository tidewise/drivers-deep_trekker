rock_library(deep_trekker
    SOURCES CommandActionMessageParser.cpp
            RustySignalMessageDecoder.cpp
            SignalRMessageDecoder.cpp
            HttpPostRequestReplyMessageDecoder.cpp
    HEADERS CommandActionMessageParser.hpp
            DeepTrekkerCommands.hpp
            DeepTrekkerStates.hpp
            RustySignalMessageDecoder.hpp
            SignalRMessageDecoder.hpp
            HttpPostRequestReplyMessageDecoder.hpp
    DEPS_PKGCONFIG jsoncpp base-types)

rock_executable(deep_trekker_bin Main.cpp
    DEPS deep_trekker
    DEPS_PKGCONFIG libdatachannel curlpp)
