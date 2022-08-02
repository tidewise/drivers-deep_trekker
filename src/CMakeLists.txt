rock_library(deep_trekker
    SOURCES RustySignalMessageDecoder.cpp
    HEADERS RustySignalMessageDecoder.hpp
    DEPS_PKGCONFIG jsoncpp)

rock_executable(deep_trekker_bin Main.cpp
    DEPS deep_trekker
    DEPS_PKGCONFIG libdatachannel curlpp)

