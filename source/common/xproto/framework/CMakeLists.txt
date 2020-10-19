cmake_minimum_required(VERSION 2.8)

# project name
project(xproto)

# add header files path
include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

get_filename_component(XPROTO_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
#message("xproto current dir init:${XPROTO_CURRENT_DIR}")
set(RELEASE_ROOT_DIR)
function(get_parent_dir arg dir n)
  foreach(i RANGE ${ARGV2})
    get_filename_component(dir ${dir} DIRECTORY)
    #message("i:${i} dir:${dir}") 
  endforeach()
  set(RELEASE_ROOT_DIR ${dir} PARENT_SCOPE)
endfunction()
get_parent_dir(XPROTO_CURRENT_DIR ${XPROTO_CURRENT_DIR} 2)
set(XSTREAM_CURRENT_DIR ${RELEASE_ROOT_DIR}/source/common/xstream)
#message("xproto current dir:${XPROTO_CURRENT_DIR}")
#message("release root dir:${RELEASE_ROOT_DIR}")
message("xstream current dir:${XSTREAM_CURRENT_DIR}")
option(X86_ARCH "x86 architecture" OFF)
option(X86_CENTOS "centos release" OFF)

# x86 architecture
if (${X86_ARCH})
  # centos release
  if (${X86_CENTOS})
    message("tool chain:${CMAKE_CXX_COMPILER}")
    add_definitions(-std=c++11)
    include_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/include
    )
  else()
    message("tool chain:${CMAKE_CXX_COMPILER}")
    add_definitions(-std=c++11)
    include_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/include
    )
  endif()
# arm architecture
else ()
  set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
  message("tool chain:${CMAKE_CXX_COMPILER}")
  include_directories(
    ${RELEASE_ROOT_DIR}/deps/hobotlog/include
  )
endif()

# header files
set(PROJECT_INC
  "include/xproto/manager/msg_manager.h"
  "include/xproto/message/pluginflow/flowmsg.h"
  "include/xproto/message/pluginflow/msg_registry.h"
  "include/xproto/plugin/xplugin.h"
  "include/xproto/plugin/xpluginasync.h"
  "include/xproto/threads/threadpool.h"
)

# source files
set(PROJECT_SRC
  "src/message/pluginflow/msg_registry.cpp"
  "src/plugin/xplugin.cpp"
  "src/plugin/xpluginasync.cpp"
  "src/threads/threadpool.cpp"
  "src/utils/gtest_helper.cpp"
  "src/utils/time_helper.cpp"
)

set(SOURCE_FILES
  ${PROJECT_SRC}
  ${PROJECT_INC}
)

set(BUILD_SHARED_LIBS true)
add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(test)
add_subdirectory(sample)

# make install
set(OUTPUT_ROOT ${PROJECT_SOURCE_DIR}/output)
set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/xproto)
install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/build/lib/
        DESTINATION ${MY_OUTPUT_ROOT}/lib/)
install(DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/xproto/
        DESTINATION ${MY_OUTPUT_ROOT}/include/xproto)
