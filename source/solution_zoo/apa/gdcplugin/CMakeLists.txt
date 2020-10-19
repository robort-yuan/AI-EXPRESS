cmake_minimum_required(VERSION 2.8)

# 项目名称
project(gdcplugin)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PARENT_DIR}/multismartplugin/include
)

# 头文件路径信息
file(GLOB PROJECT_INC
    "include/gdcplugin/*.h"
)

file(GLOB PROJECT_SRC
    "src/gdcplugin/*.cpp")

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INC})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set(BASE_LIBRARIES xproto xproto_msgtype xstream-framework xstream-imagetools hobotlog jsoncpp pthread)

target_link_libraries(${PROJECT_NAME} stitch_image ${BASE_LIBRARIES})

add_subdirectory(test)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)
# install configs, some scripts will depends this
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/config/
    DESTINATION ${MY_OUTPUT_ROOT}/config/)

