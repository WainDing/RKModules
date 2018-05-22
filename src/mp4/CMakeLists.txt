include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mp4v2/${ARCH}/inc)
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mp4v2/${ARCH}/lib)

set(MP4_SRC mp4.cpp)

add_library(rkmedia_mp4 STATIC ${MP4_SRC})
target_link_libraries(rkmedia_mp4 mp4v2)
