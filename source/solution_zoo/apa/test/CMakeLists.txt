cmake_minimum_required(VERSION 2.8)
set(target_name ${PROJECT_NAME}_ipm_test)

set(SOURCE_FILES
    main.cpp
)

add_executable(${target_name} ${SOURCE_FILES})

set(BASE_LIBRARIES bpu_predict cnn_intf hbrt_bernoulli_aarch64 cam vio
    opencv_world viowrapper 
    hobotlog 
    PredictMethod PostProcessMethod jsoncpp
)
set(XSTREAM_LB -Wl,--whole-archive xstream-framework -Wl,--no-whole-archive)

target_link_libraries(${target_name} ${BASE_LIBRARIES}
    ${XSTREAM_LB}
    -lpthread -ldl -lrt)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

# MESSAGE( STATUS "this var key = ${PROJECT_NAME}.")
install(TARGETS ${target_name}
        DESTINATION ${MY_OUTPUT_ROOT}/)
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/ipm_6001.nv12
        DESTINATION ${MY_OUTPUT_ROOT}/configs/)