include_directories(${PROJECT_SOURCE_DIR}/include)

set(CAM_SRC cam.cpp)

add_library(rkmedia_cam STATIC ${CAM_SRC})

add_subdirectory(test)