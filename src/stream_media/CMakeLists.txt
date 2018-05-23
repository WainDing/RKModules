include_directories(${PROJECT_SOURCE_DIR}/include)

set(STREAM_SRC crtp.c stream_media.cpp)

add_library(rkmedia_stream STATIC ${STREAM_SRC})
