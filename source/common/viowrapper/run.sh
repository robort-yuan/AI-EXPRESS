#!/bin/sh

usage() {
  echo "usage: sh run.sh [imx327 | os8a10 | s5kgm ] [test_num] [dump_en]"
  exit 1
}

params_num=$#
echo "params_num is $params_num"
if [ $params_num -eq 3 ];then
  sensor=$1
  test_num=$2
  dump_en=$3
  echo "sensor_name is $sensor"
  echo "test_num is $test_num"
else
  usage
fi
#enter aiexpress_root/build/bin
rm -rf libcam.so
rm -rf libvio.so
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./
config_path="./config"
if [ ! -d ${config_path} ]; then
  cp ../../common/viowrapper/config/ -R .
  cp ../../common/viowrapper/data/ -R .
fi
#cd build/bin/
# ./VIOWrapper_unit_test
if [ $sensor == "fb" ];then
   ./VIOWrapper_unit_test --gtest_filter=VIO_WRAPPER_TEST.GetFbImage_1080
else
 if [ $sensor == "s5kgm" ];then
   echo "sensor is s5kgm1sp..."
   echo start > /sys/devices/virtual/graphics/iar_cdev/iar_test_attr
   service adbd stop
   /etc/init.d/usb-gadget.sh start uvc-hid
   echo 0 > /proc/sys/kernel/printk
   echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
   echo 0xc0020000 > /sys/bus/platform/drivers/ddr_monitor/axibus_ctrl/all
   echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/read_qos_ctrl/all
   echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/write_qos_ctrl/all
 elif [ $sensor == "os8a10" ];then
   echo "sensor is os8a10..."
   echo start > /sys/devices/virtual/graphics/iar_cdev/iar_test_attr
   service adbd stop
   /etc/init.d/usb-gadget.sh start uvc-hid
   echo 0 > /proc/sys/kernel/printk
   echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
   echo 0xc0020000 > /sys/bus/platform/drivers/ddr_monitor/axibus_ctrl/all
   echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/read_qos_ctrl/all
   echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/write_qos_ctrl/all
 elif [ $sensor == "imx327" ];then
   echo "sensor is imx327..."
   echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
 fi
  ./VIOWrapper_unit_test $sensor $test_num $dump_en --gtest_filter=VIO_WRAPPER_TEST.GetSingleImage_1080
  #./VIOWrapper_unit_test $sensor $test_num $dump_en
fi
