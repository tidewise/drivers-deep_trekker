rock_library(deep_trekker
    SOURCES RustySignalMessageDecoder.cpp
            SignalRMessageDecoder.cpp
            HttpPostRequestReplyMessageDecoder.cpp
    HEADERS RustySignalMessageDecoder.hpp
            SignalRMessageDecoder.hpp
            HttpPostRequestReplyMessageDecoder.hpp
    DEPS_PKGCONFIG jsoncpp base-types)

rock_executable(deep_trekker_bin Main.cpp
    DEPS deep_trekker
    DEPS_PKGCONFIG libdatachannel curlpp)

