add_subdirectory(cam)
add_subdirectory(codec)
add_subdirectory(mp4)
add_subdirectory(stream_media)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/inc)

link_directories(${PROJECT_SOURCE_DIR}/src/cam)
link_directories(${PROJECT_SOURCE_DIR}/src/codec)
link_directories(${PROJECT_SOURCE_DIR}/src/mp4)
link_directories(${PROJECT_SOURCE_DIR}/src/stream_media)
link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/lib)

set(SRC cam/cam.cpp
        codec/codec.cpp
        stream_media/stream_media.cpp
        mp4/mp4.cpp)

add_library(rkmedia SHARED ${SRC})
target_link_libraries(rkmedia pthread rkmedia_cam rkmedia_codec rkmedia_mp4 rkmedia_stream rockchip_mpp)