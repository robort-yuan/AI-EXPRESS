cmake_minimum_required(VERSION 2.8)

include_directories(
  .
)
set(SOURCE_FILES method_factory.cpp method/MultiSourceMethod.cpp)

SET(BASE_LIBRARIES xstream jsoncpp hobotlog rt pthread)
add_executable(stage4_multisource multisource_main.cpp ${SOURCE_FILES})
target_link_libraries(stage4_multisource ${BASE_LIBRARIES})

