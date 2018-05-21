include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/src/cam)

set(CAM_TEST cam_test.cpp)
add_executable(cam_test ${CAM_TEST})
target_link_libraries(cam_test rkmedia_cam)