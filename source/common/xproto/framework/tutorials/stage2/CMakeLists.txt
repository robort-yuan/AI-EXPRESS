cmake_minimum_required(VERSION 2.8)

# 设置头文件搜索路径
include_directories(include)

set(BASE_LIBRARIES  xproto rt pthread hobotlog)

link_directories(
  ${LIB_AIEXRESS_DIR}/lib
)

set(SOURCE_FILES
        stage2_sample_plugin.cpp
)

add_executable(stage2_sample_plugin ${SOURCE_FILES})
target_link_libraries(stage2_sample_plugin ${BASE_LIBRARIES})
