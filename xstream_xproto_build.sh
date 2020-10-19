#!/bin/bash

function usage(){
  echo "usage: bash xstream_xproto_build.sh [ ubuntu | centos | aarch64 ]"
  exit 1
}

function init(){
  set -eux
  if [ $# -lt 1 ];then
    usage
  fi
  rm -rf xstream_xproto_build
  rm -rf lib_aiexpress
}

function build_ubuntu(){
  mkdir -p xstream_xproto_build
  cd xstream_xproto_build
  mkdir -p xstream
  cd xstream
  cmake -DX86_ARCH=ON -DX86_CENTOS=OFF ../../source/common/xstream/framework && make -j
  cd ..
  mkdir -p xproto
  cd xproto
  cmake -DX86_ARCH=ON -DX86_CENTOS=OFF ../../source/common/xproto/framework && make -j
  cd ../..
}

function build_centos(){
  mkdir -p xstream_xproto_build
  cd xstream_xproto_build
  mkdir -p xstream
  cd xstream
  cmake -DX86_ARCH=ON -DX86_CENTOS=ON ../../source/common/xstream/framework && make -j
  cd ..
  mkdir -p xproto
  cd xproto
  cmake -DX86_ARCH=ON -DX86_CENTOS=ON ../../source/common/xproto/framework && make -j
  cd ../..
}

function build_aarch64(){
  mkdir -p xstream_xproto_build
  cd xstream_xproto_build
  mkdir -p xstream
  cd xstream
  cmake -DX86_ARCH=OFF ../../source/common/xstream/framework && make -j
  cd ..
  mkdir -p xproto
  cd xproto
  cmake -DX86_ARCH=OFF ../../source/common/xproto/framework && make -j
  cd ../..
}

function build(){
  if [ ${1} == "ubuntu" ];then
    build_ubuntu
  elif [ ${1} == "centos" ];then
    build_centos
  elif [ ${1} == "aarch64" ];then
    build_aarch64
  else
    usage
  fi
}

function copy_xstream_xproto() {
  mkdir -p lib_aiexpress/include/xstream/include
  mkdir -p lib_aiexpress/include/xproto/include/xproto/message
  mkdir -p lib_aiexpress/lib
  
  cp xstream_xproto_build/xstream/libxstream.so lib_aiexpress/lib
  cp -rf source/common/xstream/framework/include/hobotxsdk lib_aiexpress/include/xstream/include
  cp -rf source/common/xstream/framework/include/hobotxstream lib_aiexpress/include/xstream/include
  cp xstream_xproto_build/xproto/libxproto.so lib_aiexpress/lib
  cp -rf source/common/xproto/framework/include/xproto/message/pluginflow lib_aiexpress/include/xproto/include/xproto/message
  cp -rf source/common/xproto/framework/include/xproto/plugin lib_aiexpress/include/xproto/include/xproto
  cp -rf source/common/xproto/framework/include/xproto/threads lib_aiexpress/include/xproto/include/xproto

  rm -rf xstream_xproto_build
}

function copy_third_party() {
  mkdir -p lib_aiexpress/third_party/aarch64
  cp -rf deps/gtest/ lib_aiexpress/third_party/aarch64
  cp -rf deps/hobotlog/ lib_aiexpress/third_party/aarch64
  cp -rf deps/jsoncpp/ lib_aiexpress/third_party/aarch64

  mkdir -p lib_aiexpress/third_party
  cp -rf source/common/xstream/framework/third_party lib_aiexpress/
}

function copy_xstream_xproto_example() {
  mkdir -p lib_aiexpress/example/xstream
  cp -rf source/common/xstream/tutorials/stage1/*_external lib_aiexpress/example/xstream/
  cp -rf source/common/xstream/tutorials/stage1/filter_param.h lib_aiexpress/example/xstream/
  cp -rf source/common/xstream/tutorials/stage1/method_factory.h lib_aiexpress/example/xstream/
  cp -rf source/common/xstream/tutorials/stage1/config lib_aiexpress/example/xstream/
  mv lib_aiexpress/example/xstream/CMakeLists.txt_external lib_aiexpress/example/xstream/CMakeLists.txt
  mv lib_aiexpress/example/xstream/build.sh_external lib_aiexpress/example/xstream/build.sh
  mv lib_aiexpress/example/xstream/sync_main.cc_external lib_aiexpress/example/xstream/sync_main.cc
  mv lib_aiexpress/example/xstream/async_main.cc_external lib_aiexpress/example/xstream/async_main.cc
  mv lib_aiexpress/example/xstream/update_param_main.cc_external lib_aiexpress/example/xstream/update_param_main.cc
  mv lib_aiexpress/example/xstream/method_factory.cc_external lib_aiexpress/example/xstream/method_factory.cc
  mv lib_aiexpress/example/xstream/callback.h_external lib_aiexpress/example/xstream/callback.h
  mv lib_aiexpress/example/xstream/method_external lib_aiexpress/example/xstream/method
  mv lib_aiexpress/example/xstream/README.md_external lib_aiexpress/example/xstream/README.md

  mkdir -p lib_aiexpress/example/xproto
  cp -rf source/common/xproto/framework/sample/CMakeLists.txt_external lib_aiexpress/example/xproto/CMakeLists.txt
  cp -rf source/common/xproto/framework/sample/sample_plugin.cpp lib_aiexpress/example/xproto/sample_plugin.cpp
  cp -rf source/common/xproto/framework/sample/build.sh_external lib_aiexpress/example/xproto/build.sh
  cp -rf source/common/xproto/framework/sample/README.md_external lib_aiexpress/example/xproto/README.md
}

init ${1}
build ${1}
copy_xstream_xproto ${1}
copy_third_party ${1}
copy_xstream_xproto_example ${1}

