cmake_minimum_required(VERSION 2.8)
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
)

# x86 architecture
if (${X86_ARCH})
  if (${X86_CENTOS})
    include_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/include/
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/jsoncpp/include/
      ${RELEASE_ROOT_DIR}/deps/opencv/include/
      ${XSTREAM_CURRENT_DIR}/vision_type/include/
    )
    link_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/lib
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/jsoncpp/lib/
    )
  else ()
    include_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/include/
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/jsoncpp/include/
      ${RELEASE_ROOT_DIR}/deps/opencv/include/
      ${XSTREAM_CURRENT_DIR}/vision_type/include/
    )
    link_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/lib
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/jsoncpp/lib/
    )
  endif ()
else ()
  include_directories(
    ${RELEASE_ROOT_DIR}/deps/jsoncpp/include/
    ${RELEASE_ROOT_DIR}/deps/hobotlog/include/
    ${RELEASE_ROOT_DIR}/deps/opencv/include/
    ${XSTREAM_CURRENT_DIR}/vision_type/include/
  )
  link_directories(
    ${RELEASE_ROOT_DIR}/deps/hobotlog/lib/
    ${RELEASE_ROOT_DIR}/deps/jsoncpp/lib/
  )
endif ()

set(BASE_LIBRARIES xstream hobotlog jsoncpp pthread dl)

set(SOURCE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/src/method/bbox_filter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/method_factory.cpp
)
file(GLOB_RECURSE SOURCE_SRC "src/data_types/*.cpp" "src/data_types/*.cc")

add_executable(bbox_filter_example src/bbox_filter_main.cpp)
target_link_libraries(bbox_filter_example 
  PUBLIC ${BASE_LIBRARIES}
)

if(C_INTERFACE)
    add_executable(c_bbox_filter_example src/c_bbox_filter_main.c)
    target_link_libraries(c_bbox_filter_example 
      PUBLIC ${BASE_LIBRARIES}
    )
endif()

add_executable(profiler_test src/profiler_test.cpp)
target_link_libraries(profiler_test 
  PUBLIC ${BASE_LIBRARIES}
)

add_executable(disable_method_test src/disable_method_test.cpp)
target_link_libraries(disable_method_test 
  PUBLIC ${BASE_LIBRARIES}
)
