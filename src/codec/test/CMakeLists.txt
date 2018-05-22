include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/inc)

link_directories(${PROJECT_SOURCE_DIR}/src/codec)
link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/lib)

set(CODEC_TEST codec_test.cpp)
add_executable(codec_test ${CODEC_TEST})
target_link_libraries(codec_test rkmedia_codec)
