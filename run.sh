#!/bin/sh
usage() {
   echo "usage: sh run.sh "
   exit 1
}

chmod +x start_nginx.sh
sh start_nginx.sh

if [ ! -L "./lib/libgomp.so.1" ];then
  ln -s /lib/libgomp.so ./lib/libgomp.so.1
  echo "create symbolic link in ./lib directory"
fi

export LD_LIBRARY_PATH=./lib/
vio_cfg_file=./configs/vio_config.json.96board
vio_pipe_path="configs/vio/x3dev/"
vio_pipe_pre="iot_vio_x3_"
sensor1="imx327"
sensor2="os8a10"
vio_cfg_name="./configs/vio_config.json"
vio_mode="panel_camera"
cam_mode="panel_camera"

#default solution mode is body in ut mode
solution_mode="body"
solution_mode_aNum=3
#default platform is x3dev in ut mode
platform="x3dev"
platform_aNum=4
#default vio type is fb in ut mode
vio_type="fb"
vio_type_aNum=2
#default fb_mode is cached_image_list in ut mode
fb_mode_aNum=1
fb_mode="cached_image_list"
#default sensor is os8a10 in ut mode
sensor="os8a10"
sensor_aNum=2
vio_pipe_file="${vio_pipe_path}${vio_pipe_pre}${sensor}.json"

if [ $# -gt 1 ]
then
  if [ $2 != "ut" ];then
    usage
  else
    run_mode="ut"
    echo "enter vio solution ut mode!!!"
    if [ -n "$3" ];then
      solution_mode_aNum=$3
      echo "solution_mode_aNum is $solution_mode_aNum"
    fi
    if [ -n "$4" ];then
      platform_aNum=$4
      echo "platform_aNum is $platform_aNum"
    fi
    if [ -n "$5" ];then
      vio_type_aNum=$5
      echo "vio_type_aNum is $vio_type_aNum"
    fi
    if [ $vio_type == "cam" ];then
      if [ -n "$6" ];then
        sensor_aNum=$6
        echo "sensor_aNum is $sensor_aNum"
      fi
    elif [ $vio_type == "fb" ];then
      if [ -n "$6" ];then
        fb_mode_aNum=$6
        echo "fb_mode_aNum is $fb_mode_aNum"
      fi
    else
      echo "not support vio_type:$vio_type in ut mode"
      exit 1
    fi
  fi
fi

function stop_nginx(){
  nginx_flag=$(ps | grep nginx | grep -v "grep")
  if [ -n "${nginx_flag}" ]; then
    killall -9 nginx
  fi
}

visual_cfg_func() {
  layer=$1
  width=$2
  height=$3
  layer_720p=$4
  layer_1080p=$5
  layer_2160p=$6
  sed -i 's#\("layer": \).*#\1'$layer',#g' configs/visualplugin_face.json
  sed -i 's#\("layer": \).*#\1'$layer',#g' configs/visualplugin_body.json
  sed -i 's#\("layer": \).*#\1'$layer',#g' configs/visualplugin_vehicle.json
  sed -i 's#\("image_width": \).*#\1'$width',#g' configs/visualplugin_face.json
  sed -i 's#\("image_width": \).*#\1'$width',#g' configs/visualplugin_body.json
  sed -i 's#\("image_width": \).*#\1'$width',#g' configs/visualplugin_vehicle.json
  sed -i 's#\("image_height": \).*#\1'$height',#g' configs/visualplugin_face.json
  sed -i 's#\("image_height": \).*#\1'$height',#g' configs/visualplugin_body.json
  sed -i 's#\("image_height": \).*#\1'$height',#g' configs/visualplugin_vehicle.json
  sed -i 's#\("720p_layer": \).*#\1'$layer_720p',#g' configs/visualplugin_body.json
  sed -i 's#\("1080p_layer": \).*#\1'$layer_1080p',#g' configs/visualplugin_body.json
  sed -i 's#\("2160p_layer": \).*#\1'$layer_2160p',#g' configs/visualplugin_body.json
}

solution_cfg_func() {
  resolution=$1
  cp -rf body_solution/configs/box_filter_config_${resolution}M.json  body_solution/configs/box_filter_config.json
  cp -rf body_solution/configs/guesture_multitask_${resolution}M.json  body_solution/configs/guesture_multitask.json
  cp -rf body_solution/configs/multitask_config_${resolution}M.json  body_solution/configs/multitask_config.json
  cp -rf body_solution/configs/multitask_with_hand_${resolution}M.json  body_solution/configs/multitask_with_hand.json
  cp -rf body_solution/configs/multitask_with_hand_960x544_${resolution}M.json  body_solution/configs/multitask_with_hand_960x544.json
  cp -rf body_solution/configs/multitask_with_hand_without_kps_mask_${resolution}M.json  body_solution/configs/multitask_with_hand_without_kps_mask.json
  cp -rf body_solution/configs/multitask_with_hand_without_mask_${resolution}M.json  body_solution/configs/multitask_with_hand_without_mask.json
  cp -rf body_solution/configs/segmentation_multitask_${resolution}M.json  body_solution/configs/segmentation_multitask.json
  cp -rf face_solution/configs/face_pose_lmk_${resolution}M.json face_solution/configs/face_pose_lmk.json
  cp -rf body_solution/configs/tv_dance_no_kps_mask_box_filter_config_${resolution}M.json  body_solution/configs/tv_dance_no_kps_mask_box_filter_config.json
  cp -rf body_solution/configs/tv_dance_with_kps_no_mask_box_filter_config_${resolution}M.json  body_solution/configs/tv_dance_with_kps_no_mask_box_filter_config.json
  cp -rf vehicle_solution/configs/vehicle_multitask_${resolution}M.json vehicle_solution/configs/vehicle_multitask.json
  cp -rf vehicle_solution/configs/config_match_${resolution}M.json vehicle_solution/configs/config_match.json
}

set_fb_cam_data_source_func() {
  vio_cfg_file=$1
  fb_mode=$2
  cam_mode=$3
  data_source_num=$4

  for i in $(seq 1 $data_source_num); do
    if [ $i == 1 ];then
      vio_mode=$fb_mode
    elif [ $i == 2 ];then
      vio_mode=$cam_mode
    fi
    data_src_line=`cat -n ${vio_cfg_file} | grep -w "data_source" | awk '{print $'$i'}' | sed -n ''$i'p'`
    sed -i ''${data_src_line}'s#\("data_source": \).*#\1"'${vio_mode}'",#g' ${vio_cfg_file}
  done
}

set_data_source_func() {
  vio_cfg_file=$1
  vio_mode=$2
  data_source_num=$3

  for i in $(seq 1 $data_source_num); do
    data_src_line=`cat -n ${vio_cfg_file} | grep -w "data_source" | awk '{print $1}' | sed -n ''$i'p'`
    #echo "i:$i vio_cfg_file:$vio_cfg_file data_src_line:$data_src_line vio_mode:$vio_mode"
    sed -i ''${data_src_line}'s#\("data_source": \).*#\1"'${vio_mode}'",#g' ${vio_cfg_file}
  done
}

set_cam_pipe_file_func() {
  vio_mode=$1
  sensor=$2
  vio_pipe_file=${vio_pipe_path}${vio_pipe_pre}${sensor}.json
  echo "vio_mode: $vio_mode"
  echo "vio_pipe_file: $vio_pipe_file"
  sed -i 's#\("'${vio_mode}'": \).*#\1"'${vio_pipe_file}'",#g' $vio_cfg_file
}

set_multi_cam_async_pipe_file_func() {
  index=0
  for i in "$@"; do
    sensor=$i
    index=$(($index+1))
    num=$(($index*2))
    sensor_line=`cat -n ${vio_cfg_file} | grep -w "${vio_mode}" | awk '{print $1}' | sed -n ''$num'p'`
    pipe_file="${vio_pipe_path}${vio_pipe_pre}${sensor}.json"
    echo "sensor:$sensor index:$index num:$num sensor_line:$sensor_line vio_pipe_file:$pipe_file"
    sed -i ''${sensor_line}'s#\("'$vio_mode'": \).*#\1"'${pipe_file}'",#g' ${vio_cfg_file}
  done
}

set_multi_cam_sync_pipe_file_func() {
  index=0
  vio_mode_line=`cat -n ${vio_cfg_file} | grep -w "${vio_mode}" | awk '{print $1}' | sed -n '2p'`
  for i in "$@"; do
    echo "sensor is $i"
    sensor=$i
    index=$(($index+1))
    sensor_line=$(($vio_mode_line+$index))
    pipe_file="${vio_pipe_path}${vio_pipe_pre}${sensor}.json"
    sed -i ''${sensor_line}'s#[^ ].*#"'${pipe_file}'",#g' ${vio_cfg_file}
  done
}

sensor_setting_func() {
  platform=$1
  sensor=$2
  vio_pipe_file=$3
  vin_vps_mode=$4
  i2c_bus=$5
  mipi_index=$6
  sensor_port=$7

  param_num=$#
  if [ $param_num -eq 3 ];then
    # set defalut sensor setting
    if [ $sensor == "imx327" ]; then
      # set vin_vps_mode and sensor port
      sed -i 's#\("vin_vps_mode": \).*#\1'0',#g' $vio_pipe_file
      sed -i 's#\("sensor_port": \).*#\1'0',#g' $vio_pipe_file
      # set i2c5 bus , mipi host1, sensor_port
      sed -i 's#\("i2c_bus": \).*#\1'5',#g' $vio_pipe_file
      sed -i 's#\("host_index": \).*#\1'1',#g' $vio_pipe_file
      echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
    elif [ $sensor == "os8a10" -o $sensor == "os8a10_1080p" ]; then
      # set vin_vps_mode and sensor port
      sed -i 's#\("vin_vps_mode": \).*#\1'1',#g' $vio_pipe_file
      sed -i 's#\("sensor_port": \).*#\1'0',#g' $vio_pipe_file
      if [ $platform == "x3dev" ]; then
        # set i2c5 bus and mipi host1
        sed -i 's#\("i2c_bus": \).*#\1'5',#g' $vio_pipe_file
        sed -i 's#\("host_index": \).*#\1'1',#g' $vio_pipe_file
      elif [ $platform == "x3sdb" ]; then
        # set i2c2 bus and mipihost1
        sed -i 's#\("i2c_bus": \).*#\1'2',#g' $vio_pipe_file
        sed -i 's#\("host_index": \).*#\1'1',#g' $vio_pipe_file
      fi
    elif [ $sensor == "s5kgm1sp" -o $sensor == "s5kgm1sp_2160p" ]; then
      # set vin_vps_mode and sensor port
      sed -i 's#\("vin_vps_mode": \).*#\1'0',#g' $vio_pipe_file
      sed -i 's#\("sensor_port": \).*#\1'0',#g' $vio_pipe_file
      if [ $platform == "x3dev" ]; then
        # set i2c4 bus and mipi host1
        sed -i 's#\("i2c_bus": \).*#\1'4',#g' $vio_pipe_file
        sed -i 's#\("host_index": \).*#\1'1',#g' $vio_pipe_file
        echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
      elif [ $platform == "x3sdb" ]; then
        # set i2c2 bus and mipi host1
        sed -i 's#\("i2c_bus": \).*#\1'2',#g' $vio_pipe_file
        sed -i 's#\("host_index": \).*#\1'1',#g' $vio_pipe_file
        echo 1 > /sys/class/vps/mipi_host1/param/stop_check_instart
      fi
    fi
  fi

  # set new sensor settings
  if [ -n "$i2c_bus" ];then
    echo "i2c_bus:$i2c_bus"
    sed -i 's#\("i2c_bus": \).*#\1'$i2c_bus',#g' $vio_pipe_file
  fi
  if [ -n "$mipi_index" ];then
    echo "mipi_index:$mipi_index"
    sed -i 's#\("host_index": \).*#\1'$mipi_index',#g' $vio_pipe_file
    if [ $sensor == "imx327" ];then
      echo 1 > /sys/class/vps/mipi_host${mipi_index}/param/stop_check_instart
    fi
  fi
  if [ -n "$vin_vps_mode" ];then
    echo "vin_vps_mode:$vin_vps_mode"
    sed -i 's#\("vin_vps_mode": \).*#\1'$vin_vps_mode',#g' $vio_pipe_file
  fi
  if [ -n "$sensor_port" ];then
    echo "sensor_port:$sensor_port"
    sed -i 's#\("sensor_port": \).*#\1'$sensor_port',#g' $vio_pipe_file
  fi

}

sensor_cfg_func() {
  platform=$1
  sensor=$2
  echo "platform is $platform"
  if [ $sensor == "imx327" ]; then
    echo "sensor is imx327, default sensor output resolution 1080P, 1080P X3 JPEG Codec..."
    visual_cfg_func 0 1920 1080 -1 0 -1
    solution_cfg_func 2
  elif [ $sensor == "os8a10" -o $sensor == "os8a10_1080p" ]; then
    if [ $sensor == "os8a10" ];then
      echo "sensor is os8a10, default resolution 8M, 1080P X3 JPEG Codec..."
      visual_cfg_func 4 1920 1080 5 4 0
      solution_cfg_func 8
    elif [ $sensor == "os8a10_1080p" ];then
      echo "sensor is os8a10_1080p, default resolution 1080P, 1080P X3 JPEG Codec..."
      visual_cfg_func 0 1920 1080 -1 0 -1
      solution_cfg_func 2
    fi
    echo start > /sys/devices/virtual/graphics/iar_cdev/iar_test_attr
    service adbd stop
    /etc/init.d/usb-gadget.sh start uvc-hid
    echo 0 > /proc/sys/kernel/printk
    echo 0xc0120000 > /sys/bus/platform/drivers/ddr_monitor/axibus_ctrl/all
    echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/read_qos_ctrl/all
    echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/write_qos_ctrl/all
    if [ $platform == "x3dev" ]; then
      echo ""
    elif [ $platform == "x3sdb" ]; then
      # reset os8a10 mipi cam
      echo 111 > /sys/class/gpio/export
      echo out > /sys/class/gpio/gpio111/direction
      echo 0 > /sys/class/gpio/gpio111/value
      sleep 0.2
      echo 1 > /sys/class/gpio/gpio111/value
      # enable mclk output to os8a10 sensor in x3sdb(mipihost0)
      echo 1 > /sys/class/vps/mipi_host0/param/snrclk_en
      echo 24000000 > /sys/class/vps/mipi_host0/param/snrclk_freq
      # enable mclk output to os8a10 sensor in x3sdb(mipihost1)
      echo 1 > /sys/class/vps/mipi_host1/param/snrclk_en
      echo 24000000 > /sys/class/vps/mipi_host1/param/snrclk_freq
    fi
  elif [ $sensor == "s5kgm1sp" -o $sensor == "s5kgm1sp_2160p" ]; then
    if [ $sensor == "s5kgm1sp" ];then
      echo "sensor is s5kgm1sp, default resolution 12M, 1024*768 X3 JPEG Codec..."
      visual_cfg_func 5 1024 768 -1 -1 -1
      solution_cfg_func 12
    elif [ $sensor == "s5kgm1sp_2160p" ];then
      echo "sensor is s5kgm1sp, default resolution 8M, 1080P X3 JPEG Codec..."
      visual_cfg_func 4 1920 1080 5 4 0
      solution_cfg_func 8
    fi
    echo start > /sys/devices/virtual/graphics/iar_cdev/iar_test_attr
    service adbd stop
    /etc/init.d/usb-gadget.sh start uvc-hid
    echo 0 > /proc/sys/kernel/printk
    echo 0xc0020000 > /sys/bus/platform/drivers/ddr_monitor/axibus_ctrl/all
    echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/read_qos_ctrl/all
    echo 0x03120000 > /sys/bus/platform/drivers/ddr_monitor/write_qos_ctrl/all
  elif [ $sensor == "usb_cam_1080p" ]; then
    echo "usb_cam start, default resolution 1080P..."
    service adbd stop
    solution_cfg_func 2
    visual_cfg_func 0 1920 1080 -1 0 -1
  else
    echo "error! sensor" $sensor "is not supported"
  fi
}

fb_cfg_func() {
  platform=$1
  fb_res=$2
  vio_pipe_file=$3
  echo "platform is $platform"
  echo "fb_res is $fb_res"
  echo "vio_cfg_file is $vio_cfg_file"
  echo "vio_pipe_file is $vio_pipe_file"
  echo "feedback start, default resolution 1080P, 1080P X3 JPEG Codec..."
  service adbd stop
  /etc/init.d/usb-gadget.sh start uvc-hid
  solution_cfg_func 2
  visual_cfg_func 0 1920 1080 -1 0 -1
}

choose_single_cam_func() {
  vio_cfg_file=$1
  vio_mode="panel_camera"
  data_source_num=1
  echo -e 'Choose lunch single cam sensor menu...pick a combo:'
  echo -e '\t1. single camera: imx327, default 1080P'
  echo -e '\t2. single camera: os8a10, default 2160P'
  echo -e '\t3. single camera: os8a10_1080p, default 1080P'
  echo -e '\t4. single camera: s5kgm1sp,  default 4000*3000'
  echo -e '\t5. single camera: s5kgm1sp_2160p, default 2160P'
  echo -e '\t6. single camera: usb_cam, default 1080P'
  echo -e 'Which would you like? '
  set_data_source_func ${vio_cfg_file} ${vio_mode} ${data_source_num}
  if [ x"$run_mode" == x"ut" ];then
    aNum=$sensor_aNum
  else
    read aNum
  fi
  case $aNum in
    1)  echo -e "\033[33m You choose 1:imx327 \033[0m"
      sensor=imx327
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_setting_func $platform $sensor ${vio_pipe_file}
      sensor_cfg_func $platform $sensor
      ;;
    2)  echo -e "\033[33m You choose 2:os8a10 \033[0m"
      sensor=os8a10
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_setting_func $platform $sensor ${vio_pipe_file}
      sensor_cfg_func $platform $sensor
      ;;
    3)  echo -e "\033[33m You choose 3:os8a10_1080p \033[0m"
      sensor=os8a10_1080p
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_setting_func $platform $sensor ${vio_pipe_file}
      sensor_cfg_func $platform $sensor
      ;;
    4)  echo -e "\033[33m You choose 4:s5kgm1sp \033[0m"
      sensor=s5kgm1sp
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_setting_func $platform $sensor ${vio_pipe_file}
      sensor_cfg_func $platform $sensor
      ;;
    5)  echo -e "\033[33m You choose 5:s5kgm1sp_2160p \033[0m"
      sensor=s5kgm1sp_2160p
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_setting_func $platform $sensor ${vio_pipe_file}
      sensor_cfg_func $platform $sensor
      ;;
    6)  echo -e "\033[33m You choose 6:usb_cam \033[0m"
      # vio_cfg_file=${vio_cfg_name}.x3dev.fb
      # sed -i 's#\("data_source": \).*#\1"usb_cam",#g' $vio_cfg_file
      vio_mode="usb_cam"
      sensor=usb_cam_1080p
      set_data_source_func ${vio_cfg_file} ${vio_mode} ${data_source_num}
      set_cam_pipe_file_func $vio_mode $sensor
      sensor_cfg_func $platform $sensor
      ;;
    *) echo -e "\033[31m You choose unsupport single cam mode \033[0m"
      exit
      ;;
  esac
}

choose_fb_mode_func() {
  vio_cfg_file=$1
  fb_res=$2
  data_source_num=1

  vio_pipe_file=${vio_pipe_path}${vio_pipe_pre}${fb_res}.json
  echo -e 'Choose lunch feedback source mode menu...pick a combo:'
  echo -e '\t1. cached feedback source'
  echo -e '\t2. jpeg feedback source'
  echo -e '\t3. nv12 feedback source'
  echo -e 'Which would you like? '
  if [ x"$run_mode" == x"ut" ];then
    aNum=$fb_mode_aNum
  else
    read aNum
  fi
  case $aNum in
    1)  echo -e "\033[33m You choose 1:cache \033[0m"
      fb_mode="cached_image_list"
      ;;
    2)  echo -e "\033[33m You choose 2:jpeg \033[0m"
      fb_mode="jpeg_image_list"
      sed -i 's#\("file_path": \).*#\1"configs/vio_hg/name.list",#g' $vio_cfg_file
      ;;
    3)  echo -e "\033[33m You choose 3:nv12 \033[0m"
      fb_mode="nv12_image_list"
      sed -i 's#\("file_path": \).*#\1"configs/vio_hg/name_nv12.list",#g' $vio_cfg_file
      ;;
    *) echo -e "\033[31m You choose unsupport fb mode \033[0m"
      exit
      ;;
  esac
  set_data_source_func ${vio_cfg_file} ${fb_mode} ${data_source_num}
  fb_cfg_func $platform $fb_res $vio_pipe_file
}

choose_fb_cam_func() {
  vio_cfg_file=$1
  fb_res=$2
  data_source_num=2
  cam_mode="panel_camera"

  echo -e 'Choose lunch fb_cam mode menu...pick a combo:'
  echo -e '\t1. fb_cam: fb+imx327'
  echo -e '\t2. fb_cam: fb+os8a10'
  echo -e '\t3. fb_cam: fb+os8a10_1080p'
  echo -e '\t4. fb_cam: fb+s5kgm1sp'
  echo -e '\t5. fb_cam: fb+s5kg1spm_2160p'
  echo -e 'Which would you like? '
  read aNum
  case $aNum in
    1)  echo -e "\033[33m You choose 1:fb+imx327 \033[0m"
      sensor="imx327"
      ;;
    2)  echo -e "\033[33m You choose 2:fb+os8a10 \033[0m"
      sensor="os8a10"
      ;;
    3)  echo -e "\033[33m You choose 3:fb+os8a10_1080p \033[0m"
      sensor="os8a10_1080p"
      ;;
    4)  echo -e "\033[33m You choose 4:fb+s5kgm1sp \033[0m"
      sensor="s5kgm1sp"
      ;;
    5)  echo -e "\033[33m You choose 5:fb+s5kgm1sp_2160p \033[0m"
      sensor="s5kgm1sp_2160p"
      ;;
    *) echo -e "\033[31m You choose unsupport fb_cam mode \033[0m"
      exit
      ;;
  esac
  choose_fb_mode_func ${vio_cfg_file} ${fb_res}
  vio_pipe_file=${vio_pipe_path}${vio_pipe_pre}${sensor}.json
  set_fb_cam_data_source_func ${vio_cfg_file} ${fb_mode} ${cam_mode} ${data_source_num}
  set_cam_pipe_file_func $cam_mode $sensor
  sensor_setting_func $platform $sensor ${vio_pipe_file}
  sensor_cfg_func $platform $sensor
}

set_multi_cam_setting_func() {
  echo -e 'Choose lunch multi cam sensor menu...pick a combo:'
  echo -e '\t1. multi camera: imx327+imx327'
  echo -e '\t2. multi camera: os8a10+os8a10'
  echo -e '\t3. multi camera: imx327+os8a10'
  echo -e 'Which would you like? '
  read aNum
  case $aNum in
    1)  echo -e "\033[33m You choose 1:imx327+imx327 \033[0m"
      sensor1="imx327"
      sensor2="imx327_s1"
      vin_vps_mode=2
      vio_pipe1_file=${vio_pipe_path}${vio_pipe_pre}${sensor1}.json
      vio_pipe2_file=${vio_pipe_path}${vio_pipe_pre}${sensor2}.json
      if [ ! -f $vio_pipe2_file ];then
        cp ${vio_pipe1_file} ${vio_pipe2_file}
      fi
      #imx327 sensor1 settings
      i2c_bus=5
      mipi_index=1
      sensor_port=0
      sensor_setting_func $platform $sensor ${vio_pipe1_file} ${vin_vps_mode} ${i2c_bus} ${mipi_index} ${sensor_port}
      #imx327 sensor2 settings
      i2c_bus=0
      mipi_index=0
      sensor_port=1
      sensor_setting_func $platform $sensor ${vio_pipe2_file} ${vin_vps_mode} ${i2c_bus} ${mipi_index} ${sensor_port}
      # imx327 sensor platform config
      sensor_cfg_func $platform "imx327"
      ;;
    2)  echo -e "\033[33m You choose 2:os8a10+os8a10 \033[0m"
      sensor1="os8a10"
      sensor2="os8a10_s1"
      vin_vps_mode=3
      vio_pipe1_file=${vio_pipe_path}${vio_pipe_pre}${sensor1}.json
      vio_pipe2_file=${vio_pipe_path}${vio_pipe_pre}${sensor2}.json
      if [ ! -f $vio_pipe2_file ];then
        cp ${vio_pipe1_file} ${vio_pipe2_file}
      fi
      #os8a10 sensor1 settings
      i2c_bus=5
      mipi_index=1
      sensor_port=0
      sensor_setting_func $platform $sensor ${vio_pipe1_file} ${vin_vps_mode} ${i2c_bus} ${mipi_index} ${sensor_port}
      #os8a10 sensor2 settings
      i2c_bus=0
      mipi_index=0
      sensor_port=1
      sensor_setting_func $platform $sensor ${vio_pipe2_file} ${vin_vps_mode} ${i2c_bus} ${mipi_index} ${sensor_port}
      # os8a10 sensor platform config
      sensor_cfg_func $platform "os8a10"
      ;;
    3)  echo -e "\033[33m You choose 3:imx327+os8a10, not good!!! \033[0m"
      # TODO
      exit
      sensor1="imx327"
      sensor2="os8a10"
      vin_vps_mode=1
      vio_pipe1_file=${vio_pipe_path}${vio_pipe_pre}${sensor1}.json
      vio_pipe2_file=${vio_pipe_path}${vio_pipe_pre}${sensor2}.json
      sensor_cfg_func $platform $sensor1
      sensor_cfg_func $platform $sensor2
      ;;
    *) echo -e "\033[31m You choose unsupport multi cam mode \033[0m"
      exit
      ;;
  esac
}

choose_multi_cam_async_func() {
  vio_cfg_file=$1
  vio_mode="panel_camera"
  data_source_num=2
  set_data_source_func ${vio_cfg_file} ${vio_mode} ${data_source_num}
  set_multi_cam_setting_func
  set_multi_cam_async_pipe_file_func ${sensor1} ${sensor2}
}

choose_multi_cam_sync_func() {
  vio_cfg_file=$1
  vio_mode="dual_panel_camera"
  data_source_num=1
  set_data_source_func ${vio_cfg_file} ${vio_mode} ${data_source_num}
  set_multi_cam_setting_func
  set_multi_cam_sync_pipe_file_func ${sensor1} ${sensor2}
}

choose_x3_viotype_func() {
  echo -e 'Choose lunch vio type menu...pick a combo:'
  echo -e '\t1. single cam'
  echo -e '\t2. single feedback'
  echo -e '\t3. multi cam sync'
  echo -e '\t4. multi cam async'
  echo -e '\t5. single cam + single feedback'
  echo -e 'Which would you like? '
  if [ x"$run_mode" == x"ut" ];then
    aNum=$vio_type_aNum
  else
    read aNum
  fi
  case $aNum in
    1)  echo -e "\033[33m You choose 1:single_cam \033[0m"
      vio_cfg_file=${vio_cfg_name}.x3dev.cam
      choose_single_cam_func ${vio_cfg_file}
      ;;
    2)  echo -e "\033[33m You choose 2:single_fb \033[0m"
      vio_cfg_file=${vio_cfg_name}.x3dev.fb
      fb_res=1080_fb
      choose_fb_mode_func ${vio_cfg_file} ${fb_res}
      ;;
    3)  echo -e "\033[33m You choose 3:multi_cam_sync \033[0m"
      vio_cfg_file=${vio_cfg_name}.x3dev.multi_cam_sync
      choose_multi_cam_sync_func ${vio_cfg_file}
      ;;
    4)  echo -e "\033[33m You choose 4:multi_cam_async \033[0m"
      vio_cfg_file=${vio_cfg_name}.x3dev.multi_cam_async
      choose_multi_cam_async_func ${vio_cfg_file}
      ;;
    5)  echo -e "\033[33m You choose 5:fb_cam \033[0m"
      vio_cfg_file=${vio_cfg_name}.x3dev.fb_cam
      fb_res=1080_fb
      choose_fb_cam_func ${vio_cfg_file} ${fb_res}
      ;;
    *) echo -e "\033[31m You choose unsupport vio type mode \033[0m"
      exit
      ;;
  esac
}

choose_platform_func() {
  pass_viotype_ck=$1
  echo "pass_viotype_ck is ${pass_viotype_ck}"
  echo -e 'Choose lunch platform menu...pick a combo:'
  echo -e '\t1. x2_96board'
  echo -e '\t2. x2_2610'
  echo -e '\t3. x3_sdb'
  echo -e '\t4. x3_dev'
  echo -e '\t5. j3_dev'
  echo -e 'Which would you like? '
  if [ x"$run_mode" == x"ut" ];then
    aNum=$platform_aNum
  else
    read aNum
  fi
  case $aNum in
    1)  echo -e "\033[33m You choose 1:x2 96board \033[0m"
      platform="96board"
      visual_cfg_func 4 960 540 -1 -1 -1
      vio_cfg_file=${vio_cfg_name}.96board
      ;;
    2)  echo -e "\033[33m You choose 2:x2 2610 \033[0m"
      platform="2610"
      visual_cfg_func 4 960 540 -1 -1 -1
      vio_cfg_file=${vio_cfg_name}.2610
      ;;
    3)  echo -e "\033[33m You choose 3:x3sdb \033[0m"
      platform="x3sdb"
      echo 105000 >/sys/devices/virtual/thermal/thermal_zone0/trip_point_1_temp
      echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
      # use dynamic ion alloc memory
      echo 0 > /sys/class/misc/ion/cma_carveout_size
      if [ -z "${pass_viotype_ck}" ]; then
        choose_x3_viotype_func
      fi
      ;;
    4)  echo -e "\033[33m You choose 4:x3dev \033[0m"
      platform="x3dev"
      echo 105000 >/sys/devices/virtual/thermal/thermal_zone0/trip_point_1_temp
      echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
      # use dynamic ion alloc memory
      echo 0 > /sys/class/misc/ion/cma_carveout_size
      if [ -z "${pass_viotype_ck}" ]; then
        choose_x3_viotype_func
      fi
      ;;
    5)  echo -e "\033[33m You choose 5:j3dev \033[0m"
      platform="j3dev"
      vio_cfg_file=${vio_cfg_name}.j3dev
      ;;
    *) echo -e "\033[31m You choose unsupport platform mode \033[0m"
      exit
      ;;
  esac
}

choose_solution_func() {
  log_level=$1
  if [ -z "$log_level" ];then
    log_level="i"
  fi
  echo "log_level: $log_level"
  echo "run_mode: $run_mode"
  echo -e 'Choose lunch solution menu...pick a combo:'
  echo -e '\t1.  face'
  echo -e '\t2.  face_recog'
  echo -e '\t3.  body'
  echo -e '\t4.  xbox'
  echo -e '\t5.  behavior'
  echo -e '\t6.  gesture'
  echo -e '\t7.  video_box'
  echo -e '\t8.  tv_dance'
  echo -e '\t9.  tv_dance_960x544'
  echo -e '\t10. tv_dance_no_kps_mask'
  echo -e '\t11. tv_dance_with_kps_no_mask'
  echo -e '\t12. face_body_multisource'
  echo -e '\t13. apa'
  echo -e '\t14. apa_test'
  echo -e '\t15. ssd_test'
  # echo -e '\t16. vehicle'
  # echo -e '\t17. vehicle_v2'
  # echo -e '\t18. gtest_vision_type'
  # echo -e '\t19. gtest_image_tools'
  # echo -e '\t20. gtest_grading'
  # echo -e '\t21. x2_4k'
  echo -e 'Which would you like? '
  if [ x"$run_mode" == x"ut" ];then
    aNum=$solution_mode_aNum
    echo "solution_mode_aNum is $solution_mode_aNum"
  else
    read aNum
  fi
  case $aNum in
    1)  echo -e "\033[33m You choose 1:face \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./face_solution/face_solution $vio_cfg_file ./face_solution/configs/face_solution.json ./configs/visualplugin_face.json -${log_level}
      else
        ./face_solution/face_solution $vio_cfg_file ./face_solution/configs/face_solution.json ./configs/visualplugin_face.json -${log_level} normal
      fi
      ;;
    2)  echo -e "\033[33m You choose 2:face_recog \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./face_solution/face_solution $vio_cfg_file ./face_solution/configs/face_recog_solution.json ./configs/visualplugin_face.json -${log_level}
      else
        ./face_solution/face_solution $vio_cfg_file ./face_solution/configs/face_recog_solution.json ./configs/visualplugin_face.json -${log_level} normal
      fi
      ;;
    3)  echo -e "\033[33m You choose 3:body \033[0m"
      choose_platform_func
      echo "vio_cfg_file: $vio_cfg_file"
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/body_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/body_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    4)  echo -e "\033[33m You choose 4:xbox \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/xbox_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/xbox_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    5)  echo -e "\033[33m You choose 5:behavior \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/behavior_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/behavior_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    6)  echo -e "\033[33m You choose 6:gesture \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/guesture_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/guesture_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    7)  echo -e "\033[33m You choose 7:video_box \033[0m"
      solution_cfg_func 2
      visual_cfg_func 0 1920 1080 -1 0 -1
      if [ x"$run_mode" == x"ut" ];then
        ./video_box/video_box $vio_cfg_file ./video_box/configs/body_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./video_box/video_box $vio_cfg_file ./video_box/configs/body_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    8)  echo -e "\033[33m You choose 8:tv_dance \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/dance_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/dance_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    9)  echo -e "\033[33m You choose 9:tv_dance_960x544 \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/dance_solution_multitask_960x544.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/dance_solution_multitask_960x544.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    10)  echo -e "\033[33m You choose 10:tv_dance_no_kps_mask \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/tv_dance_no_kps_mask_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/tv_dance_no_kps_mask_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    11)  echo -e "\033[33m You choose 11:tv_dance_with_kps_no_mask \033[0m"
      choose_platform_func
      if [ x"$run_mode" == x"ut" ];then
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/tv_dance_with_kps_no_mask_solution.json ./configs/visualplugin_body.json -${log_level}
      else
        ./body_solution/body_solution $vio_cfg_file ./body_solution/configs/tv_dance_with_kps_no_mask_solution.json ./configs/visualplugin_body.json -${log_level} normal
      fi
      ;;
    12)  echo -e "\033[33m You choose 12:face_body_multisouce \033[0m"
      choose_platform_func "pass_viotype_ck"
      vio_cfg_file=${vio_cfg_name}.$platform.fb
      if [ $platform == "x3sdb" ];then
        vio_cfg_file=${vio_cfg_name}.x3dev.fb
      fi
      echo "vio_cfg_file is ${vio_cfg_file}"
      if [ $platform == "x3dev" -o $platform == "x3sdb" ];then
        sed -i 's#\("data_source": \).*#\1"cached_image_list",#g' ${vio_cfg_file}
      fi
      if [ x"$run_mode" == x"ut" ];then
        ./face_body_multisource/face_body_multisource ${vio_cfg_file} ./face_body_multisource/configs/face_body_solution.json -${log_level}
      else
        ./face_body_multisource/face_body_multisource ${vio_cfg_file} ./face_body_multisource/configs/face_body_solution.json -${log_level} normal
      fi
      ;;
    13)  echo -e "\033[33m You choose 13:apa \033[0m"
      vio_cfg_file=${vio_cfg_name}.j3dev
      echo "vio_cfg_file is ${vio_cfg_file}"
      if [ x"$run_mode" == x"ut" ];then
        ./apa/apa $vio_cfg_file ./apa/configs/apa_config.json ./apa/configs/websocket_config.json ./apa/configs/gdcplugin_config.json ./apa/configs/displayplugin_config.json -${log_level}
      else
        ./apa/apa $vio_cfg_file ./apa/configs/apa_config.json ./apa/configs/websocket_config.json ./apa/configs/gdcplugin_config.json ./apa/configs/displayplugin_config.json -${log_level} normal
      fi
      ;;
    14)  echo -e "\033[33m You choose 14:apa_test \033[0m"
    ./apa/multivioplugin_test
      ;;
    15)  echo -e "\033[33m You choose 15:ssd_test \033[0m"
      export LD_LIBRARY_PATH=./lib/:../lib
      cp configs/vio/vio_onsemi0230_fb.json  ./ssd_test/config/vio_config/vio_onsemi0230_fb.json
      cd ./ssd_test
      ./ssd_method_test
      ;;
    # 16)  echo -e "\033[33m You choose 16:vehicle \033[0m"
    #   if [ x"$run_mode" == x"ut" ];then
    #     ./vehicle_solution/vehicle_solution $vio_cfg_file ./vehicle_solution/configs/vehicle_solution.json ./configs/visualplugin_vehicle.json -${log_level}
    #   else
    #     ./vehicle_solution/vehicle_solution $vio_cfg_file ./vehicle_solution/configs/vehicle_solution.json ./configs/visualplugin_vehicle.json -${log_level} normal
    #   fi
    #   ;;
    # 17)  echo -e "\033[33m You choose 17:vehicle_v2 \033[0m"
    #   if [ x"$run_mode" == x"ut" ];then
    #     ./vehicle_solution/vehicle_solution $vio_cfg_file ./vehicle_solution/configs/vehicle_solution_v2.json ./configs/visualplugin_vehicle.json -${log_level}
    #   else
    #     ./vehicle_solution/vehicle_solution $vio_cfg_file ./vehicle_solution/configs/vehicle_solution_v2.json ./configs/visualplugin_vehicle.json -${log_level} normal
    #   fi
    #   ;;
    # 18)  echo -e "\033[33m You choose 18:gtest_vision_type \033[0m"
    #   ./vision_type/gtest_vision_type
    #   ;;
    # 19)  echo -e "\033[33m You choose 19:gtest_image_tools \033[0m"
    #   ./image_tools/gtest_imagetools
    #   ;;
    # 20)  echo -e "\033[33m You choose 20:gtest_grading \033[0m"
    #   cd methods/grading/
    #   ./gtest_grading
    #   ;;
    # 21)  echo -e "\033[33m You choose 21:x2_4k \033[0m"
    #   choose_platform_func
    #   if [ $platform == "96board" ]
    #   then
    #     sed -i 's#\("x2_4k": \).*#\1"configs/vio/vio_mipi2160p.json.96board",#g' configs/vio_config.json.hg.4k
    #   elif [ $platform == "2610" ]
    #   then
    #     sed -i 's#\("x2_4k": \).*#\1"configs/vio/vio_mipi2160p.json.2610",#g' configs/vio_config.json.hg.4k
    #   else
    #     echo "not support, todo"
    #   fi
    #   if [ x"$run_mode" == x"ut" ];then
    #     ./face_body_multisource/face_body_multisource configs/vio_config.json.hg.4k ./face_body_multisource/configs/face_body_solution.json
    #   else
    #     ./face_body_multisource/face_body_multisource configs/vio_config.json.hg.4k ./face_body_multisource/configs/face_body_solution.json -w normal
    #   fi
    #   ;;
    *) echo -e "\033[31m You choose unsupport solution mode \033[0m"
      exit
      ;;
  esac
}

choose_solution_func $1
# stop_nginx
