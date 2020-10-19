cmake_minimum_required(VERSION 2.8)

project(xproto_python_api)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(GRAND_DIR ${PARENT_DIR} DIRECTORY)
message(${PARENT_DIR})
message(${GRAND_DIR})
message(${VioPlugin})
message(${CMAKE_SOURCE_DIR})

set(CMAKE_STRIP "/opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-strip")

set(ARCH_LIBS vio cam)

include_directories(
				include
				${PARENT_DIR}/framework/include
				${PARENT_DIR}/msgtype/include
				${PARENT_DIR}/msgtype/include/xproto_msgtype/protobuf
				${PARENT_DIR}/plugins/smartplugin/include
#				${PARENT_DIR}/plugins/vioplugin/include
        ${VioPlugin}/include
				${PARENT_DIR}/plugins/websocketplugin/include
				pybind11/include
        x2_prebuilt/include
        x3_prebuilt/include
        ${GRAND_DIR}/xstream/vision_type/include
        ${GRAND_DIR}/xstream/framework/include
)

file(GLOB PYMODULE_INC
				"../framework/include/xproto/plugin/*.h"
)

file(GLOB PYMODULE_SRC
				"../framework/src/plugin/*.cpp"
)

file(GLOB X2_INC "../plugins/vioplugin/include/vioplugin/*.h")
file(GLOB X2_SRC "../plugins/vioplugin/src/vioplugin/*.cpp")
file(GLOB X3_INC "../plugins/iotvioplugin/include/vioplugin/*.h")
file(GLOB X3_SRC "../plugins/iotvioplugin/src/vioplugin/*.cpp")

set(SOURCE_FILES
				${PYMODULE_INC}
				${PYMODULE_SRC}
)

if(${PLATFORM_X2})
    set(SOURCE_FILES ${SOURCE_FILES} ${X2_INC} ${X2_SRC})
endif()

if(${PLATFORM_X3})
    set(SOURCE_FILES ${SOURCE_FILES} ${X3_INC} ${X3_SRC})
endif()

set(PYBIND11_PYTHON_VERSION 3.6)
set(OpenCVLib opencv_world)
set(BASE_LIBRARIES
    websocketplugin smartplugin vioplugin
    xproto xproto_msgtype
    xstream-framework hobotsdk hobotthread
    vision_type_util hobotlog protobuf jsoncpp ${OpenCVLib} ${ARCH_LIBS})

pybind11_add_module(native_xproto
				${SOURCE_FILES}
				src/smart_helper.cpp
				src/wrapper.cpp
        src/method_factory.cpp
)

target_link_libraries(native_xproto PRIVATE ${BASE_LIBRARIES} ${DEP_LIBS})
