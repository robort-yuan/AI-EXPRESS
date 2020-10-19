cmake_minimum_required(VERSION 2.8)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

# x86 architecture
if (${X86_ARCH})
  # centos release
  if(${X86_CENTOS})
    link_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/lib
    )
  else()
    link_directories(
      ${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/lib
    )
  endif()
# arm architecture
else ()
  link_directories(
    ${RELEASE_ROOT_DIR}/deps/hobotlog/lib
  )
endif()

# all source files of sample code
set(SOURCE_FILES
  sample_plugin.cpp
)

## base deps
if (PLATFORM_WIN)
  set(BASE_LIBRARIES hobotlog.lib)
else ()
  set(BASE_DEPS -ldl -lm -lpthread -lrt)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} libhobotlog.a)
  set(BASE_LIBRARIES ${BASE_LIBRARIES} ${BASE_DEPS})
endif ()

# hobot_util.cmake已经设置了在build/bin中生成可执行文件，默认应该在sample目录里生成
add_executable(sample_plugin ${SOURCE_FILES})
target_link_libraries(sample_plugin
  PUBLIC ${PROJECT_NAME} ${BASE_LIBRARIES}
)
