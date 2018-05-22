macro(add_test module)
   set(test_name ${module})
   include_directories(${PROJECT_SOURCE_DIR}/include)
   include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/inc)
   include_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mp4v2/${ARCH}/inc)
   link_directories(${PROJECT_SOURCE_DIR}/src)
   link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mpp/${ARCH}/lib)
   link_directories(${PROJECT_SOURCE_DIR}/3rdBinarys/mp4v2/${ARCH}/lib)
   add_executable(${test_name} ${test_name}.cpp)
   target_link_libraries(${test_name} rkmedia pthread)
endmacro()

add_test(video_capture_test)

add_test(video_encode2push_rtp)

add_test(video_encode2push_rtsp)

add_test(video_encode2save_mp4)

add_test(video_encode_test)
