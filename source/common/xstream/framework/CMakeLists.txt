cmake_minimum_required(VERSION 2.8)
project(xstream-framework
	LANGUAGES CXX
)

get_filename_component(XSTREAM_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# message("xstream current dir init:${XSTREAM_CURRENT_DIR}")
set(RELEASE_ROOT_DIR)
function(get_parent_dir arg dir n)
  foreach(i RANGE ${ARGV2})
    get_filename_component(dir ${dir} DIRECTORY)
    #message("i:${i} dir:${dir}")
  endforeach(i)
  set(RELEASE_ROOT_DIR ${dir} PARENT_SCOPE)
endfunction()

get_parent_dir(XSTREAM_CURRENT_DIR ${XSTREAM_CURRENT_DIR} 2)
# message("xstream current dir:${XSTREAM_CURRENT_DIR}")
# message("source dir:${RELEASE_ROOT_DIR}")

option(X86_ARCH "x86 architecture" OFF)
option(X86_CENTOS "centos release" OFF)
option(XSTREAM_HOBOTSDK_ENGINE "choose hobotsdk engine" OFF)
option(C_INTERFACE "add c interface" OFF)
if(C_INTERFACE)
	add_definitions(-DC_INTERFACE)
endif()

# x86 architecture
if (${X86_ARCH})
  if (${X86_CENTOS})
		message("tool chain:${CMAKE_CXX_COMPILER}")
		add_definitions(-std=c++11)
		include_directories(
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/jsoncpp/include/
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/include/
			${XSTREAM_CURRENT_DIR}/vision_type/include/
		)
		link_directories(
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/centos/hobotlog/lib
		)
	else ()
		message("tool chain:${CMAKE_CXX_COMPILER}")
		add_definitions(-std=c++11)
		include_directories(
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/jsoncpp/include/
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/include/
			${XSTREAM_CURRENT_DIR}/vision_type/include/
		)
		link_directories(
			${XSTREAM_CURRENT_DIR}/framework/third_party/x86/ubuntu/hobotlog/lib
		)
	endif ()
else ()
	set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
  message("tool chain:${CMAKE_CXX_COMPILER}")
	include_directories(
		${RELEASE_ROOT_DIR}/deps/jsoncpp/include/
		${RELEASE_ROOT_DIR}/deps/hobotlog/include/
		${XSTREAM_CURRENT_DIR}/vision_type/include/
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/common
	${CMAKE_CURRENT_SOURCE_DIR}/include/log
	${CMAKE_CURRENT_SOURCE_DIR}/include/timer
)

include_directories(SYSTEM /usr/include/python2.7)

set(SOURCE_FILES
	src/common/com_func.cpp
	src/profiler.cpp
	src/xstream.cpp
	src/xstream_data.cpp
	src/xstream_config.cpp
	src/method.cpp
)
if(C_INTERFACE)
	set(SOURCE_FILES ${SOURCE_FILES} src/xstream_capi.cpp src/xstream_capi_type.cpp src/xstream_c_api.cpp)
endif()

if(XSTREAM_HOBOTSDK_ENGINE)
	add_definitions(-DHOBOTSDK_ENGINE)
	include_directories(include/hobotxstream/hobotsdk)
	set(HOBOTSDK_ENGINE_SOURCES
		include/hobotxstream/hobotsdk/BaseMessage.h
		include/hobotxstream/hobotsdk/NodeV2.h
		include/hobotxstream/hobotsdk/schedulerV2.h
		include/hobotxstream/hobotsdk/ThreadSafeMethod.h
		include/hobotxstream/hobotsdk/base_modules/CollectModule.h
		include/hobotxstream/hobotsdk/base_modules/DefaultDispatchModule.h
		include/hobotxstream/hobotsdk/base_modules/DispatchModule.h
		include/hobotxstream/hobotsdk/base_modules/MethodModule.h
		include/hobotxstream/hobotsdk/base_modules/ReorderDispatchModule.h
		src/hobotsdk/NodeV2.cpp
		src/hobotsdk/schedulerV2.cpp
		src/hobotsdk/base_modules/CollectModule.cpp
		src/hobotsdk/base_modules/DefaultDispatchModule.cpp
		src/hobotsdk/base_modules/DispatchModule.cpp
		src/hobotsdk/base_modules/MethodModule.cpp
		src/hobotsdk/base_modules/ReorderDispatchModule.cpp
	)
	set(SOURCE_FILES ${SOURCE_FILES} ${HOBOTSDK_ENGINE_SOURCES})
else()
	set(NATIVE_ENGINE_SOURCES
		src/method_manager.cpp
		src/timer/timer.cpp
		src/node.cpp
		src/scheduler.cpp
		src/thread_pool.cpp
	)
	set(SOURCE_FILES ${SOURCE_FILES} ${NATIVE_ENGINE_SOURCES})
endif()

set(BUILD_SHARED_LIBS false)
add_library(xstream-framework ${SOURCE_FILES})

set(BUILD_SHARED_LIBS true)
add_library(xstream ${SOURCE_FILES})

target_link_libraries(xstream-framework
	PUBLIC rt)
target_link_libraries(xstream
	PUBLIC rt)

if(XSTREAM_HOBOTSDK_ENGINE)
    target_link_libraries(xstream-framework
			PUBLIC hobotsdk hobotthread jsoncpp hobotlog)
    target_link_libraries(xstream
			PUBLIC hobotsdk hobotthread jsoncpp hobotlog)
else()
    target_link_libraries(xstream-framework
                          PUBLIC jsoncpp)
endif()

add_subdirectory(example/bbox_filter)
add_subdirectory(test)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/hobotxsdk
	DESTINATION ${MY_OUTPUT_ROOT}/include/
	FILES_MATCHING PATTERN "*.h"
)
install(FILES
	${PROJECT_SOURCE_DIR}/include/hobotxstream/method.h
	${PROJECT_SOURCE_DIR}/include/hobotxstream/method_factory.h
	${PROJECT_SOURCE_DIR}/include/hobotxstream/profiler.h
	DESTINATION ${MY_OUTPUT_ROOT}/include/hobotxstream
)

install(TARGETS ${PROJECT_NAME}
	DESTINATION ${MY_OUTPUT_ROOT}/lib)

