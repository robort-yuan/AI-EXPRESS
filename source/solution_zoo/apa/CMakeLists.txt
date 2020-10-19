cmake_minimum_required(VERSION 2.8)
project(apa)

link_directories(
    ${PROJECT_SOURCE_DIR}/gdcplugin/deps
    ${PROJECT_SOURCE_DIR}/displayplugin/deps/lib
    ${PROJECT_SOURCE_DIR}/canplugin/deps/lib
)

add_subdirectory(multivioplugin)
add_subdirectory(multismartplugin)
add_subdirectory(multiwebsocketplugin)
add_subdirectory(test)
add_subdirectory(gdcplugin)
add_subdirectory(displayplugin)
add_subdirectory(canplugin)
add_subdirectory(analysisplugin)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/multismartplugin/include
    ${PROJECT_SOURCE_DIR}/multivioplugin/include
    ${PROJECT_SOURCE_DIR}/multiwebsocketplugin/include
    ${PROJECT_SOURCE_DIR}/gdcplugin/include
    ${PROJECT_SOURCE_DIR}/displayplugin/include
    ${PROJECT_SOURCE_DIR}/displayplugin/deps/include
    ${PROJECT_SOURCE_DIR}/canplugin/include
    ${PROJECT_SOURCE_DIR}/canplugin/deps
    ${PROJECT_SOURCE_DIR}/canplugin/deps/include
    ${PROJECT_SOURCE_DIR}/analysisplugin/include
)

set(SOURCE_FILES
        main.cpp
        method_factory.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(BASE_LIBRARIES displayplugin multismartplugin multiwebsocketplugin gdcplugin
        PredictMethod PostProcessMethod
        FasterRCNNMethod CNNMethod bpu_predict MergeMethod SnapShotMethod GradingMethod vote_method
        MOTMethod ipc_tracking xstream-framework xstream-imagetools hobotlog
        jsoncpp opencv_world multivioplugin canplugin analysisplugin)

target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/
        DESTINATION ${MY_OUTPUT_ROOT}/configs/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/)
