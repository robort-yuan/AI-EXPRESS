cmake_minimum_required(VERSION 2.8)

# 项目名称
project(canplugin)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/deps
    ${PROJECT_SOURCE_DIR}/deps/include
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(${PROJECT_SOURCE_DIR}/deps/lib)

# 头文件路径信息
file(GLOB PROJECT_INC
    "include/canplugin/*.h"
)


file(GLOB PROJECT_SRC
    "src/canplugin/*.cpp")


set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set(BASE_LIBRARIES hobotlog  pthread  xproto_msgtype jsoncpp zmq xstream-framework xproto)

target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})


set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
# install configs, some scripts will depends this
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/config/
    DESTINATION ${MY_OUTPUT_ROOT}/config/)

