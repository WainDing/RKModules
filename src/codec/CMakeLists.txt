include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/inc)
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/lib)

set(CODEC_SRC codec.cpp)

add_library(rkmedia_codec STATIC ${CODEC_SRC})
target_link_libraries(rkmedia_codec rockchip_mpp)