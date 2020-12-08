// 'use strict';
let FPS = 30;
let timeout;
let frames = [];
let videos = [];
let AwesomeMessage = null;
let messageShowSelect = {};
let socketParameters = getURLParameter();
let pathNum = getVideoNum();
let pathNumArr  = [...new Array(pathNum).keys()];

createVideo();
changeCheckboxShow();
changeCheckboxSelect();
protobufInit();
multipathInit();

function getVideoNum() {
  let num = 1;
  let obj = {};
  const cookies = document.cookie.split('; ');
  cookies.map(item => {
    let arr = item.split('=');
    obj[arr[0]] = arr[1] * 1;
  })
  if (typeof obj.path_num !== 'undefined') {
    num = obj.path_num;
  }
  return num;
}

function createVideo() {
  let w = 100;
  let h = 100;
  let names = 'video';
  if (pathNum === 2) {
    w /= 2;
    names = 'video two';
  }
  if (pathNum === 3 || pathNum === 4) {
    w /= 2;
    h /= 2;
    names = 'video four';
  }
  if (pathNum === 5 || pathNum === 6) {
    w /= 3;
    h /= 2;
    names = 'video six';
  }
  if (pathNum === 7 || pathNum === 8) {
    w /= 3;
    h /= 3;
    names = 'video eight';
  }
  
  const videocontHtml = document.getElementById('wrapper-cnt');
  const performanceHtml = document.getElementById('performance-message');
  pathNumArr.map(i => {
    const port = 8080 + i * 2;
    const str = `
      <div class="${names}" id="video-wrap-${port}" style="width: ${w}%; height: ${h}%;">
        <div class="cont">
          <div class="cam" id="cam">
            <img class="logo1" src="../assets/images/usbcam-top.png" alt="">
            <img class="logo2" src="../assets/images/aionhorizon.png" alt="">
            <img src="" class="layer" id="video-${port}" alt="">
            <canvas class="canvas" id="canvas-${port}-1"></canvas>
            <canvas class="canvas" id="canvas-${port}-2"></canvas>
            <ul class="canvas info-panel-1" id="info-panel-${port}-1">
            </ul>
            <ul class="canvas info-panel-2" id="info-panel-${port}-2">
            </ul>
          </div>
        </div>
      </div>
    `;
    const str2 = `<li id="performance-${port}"></li>`
    videocontHtml.innerHTML += str;
    performanceHtml.innerHTML += str2;
  })
}

function getURLParameter() {
  let socketIP = getUrlQueryParameter('ws');
  let port = getUrlQueryParameter('port');
  let netId = getUrlQueryParameter('netid');
  let cameraId = getUrlQueryParameter('camera');
  let id = getUrlQueryParameter('id');

  let doc_socketIP = getUrlQueryParameter('doc_ws');
  let doc_port = getUrlQueryParameter('doc_port');
  let cam_id = getUrlQueryParameter('cam_id');

  return {
    socketIP,
    port,
    netId,
    cameraId,
    id,
    doc_socketIP,
    doc_port,
    cam_id
  }
}

function changeCheckboxSelect() {
  const performanceHtml = document.getElementById('performance');
  const messageShows = Array.from(document.querySelectorAll('.message-show'));
  messageShows.forEach(item => {
    messageShowSelect[item.getAttribute('dataType')] = item.checked;
    item.onclick = () => {
      messageShowSelect[item.getAttribute('dataType')] = item.checked;
      performanceHtml.style.display = messageShowSelect.performance ? 'block' : 'none';
    }
  })
}

function changeCheckboxShow() {
  let messageChangeV = document.querySelector('.message-v');
  let messageChangeH = document.querySelector('.message-h');
  let messageDiv = document.getElementById('message');

  messageChangeH.onclick = function () {
    messageDiv.style.display = 'block';
    this.style.display = 'none';
    messageChangeV.style.display = 'block';
  }
  messageChangeV.onclick = function () {
    messageDiv.style.display = 'none';
    this.style.display = 'none';
    messageChangeH.style.display = 'block';
  }
}

function protobufInit() {
  protobuf.load('../../protos/x3.proto', function (err, root) {
    if (err) throw err;
    AwesomeMessage = root.lookupType('x3.FrameMessage');
  });
}

function multipathInit() {
  pathNumArr.map(i => {
    const port = 8080 + 2 * i;
    videos[i] = new RenderFrame(
      { canvasId: `canvas-${port}-1` },
      { canvasId: `canvas-${port}-2` },
      `info-panel-${port}-1`,
      `info-panel-${port}-2`,
      `video-${port}`,
      `performance-${port}`
    );
    wsInit(i, port);
  })
  pathNumArr = null;
}

function wsInit(i, port) {
  let { socketIP, cameraId, id, netId } = socketParameters;
  // 部署
  hostport = document.location.host;
  socketIP = hostport.replace(/_/g, '.');
  socket = new ReconnectingWebSocket(`ws://${socketIP}:${port}`, null, { binaryType: 'arraybuffer' });
  // 本地开发用
  // let ip = 
  // // '10.64.35.114'
  // // '10.64.35.121'
  // '10.103.74.106'
  // let socket = new ReconnectingWebSocket(`ws://${ip}:${port}`, null, { binaryType: 'arraybuffer' });

  socket.onopen = function (e) {
    if (e.type === 'open') {
      let data = { filter_prefix: netId + '/' + cameraId + '/' + id };
      socket.send(JSON.stringify(data));
      console.log('opened');
    }
  };

  socket.onclose = function (e) {
    if (socket) {
      console.log(`close:${socket.url} `, e);
      socket.close();
    }
  };

  socket.onerror = function (e) {
    if (socket) {
      console.log(`error:${socket.url} `, e);
    }
  };

  socket.onmessage = function (e) {
    // console.log(e)
    if (e.data) {
      frames[i] = transformData(e.data);
    }
    delete e;
  };
  clearTimeout(timeout);
  sendMessage();
}

function sendMessage() {
  videos.map((item, index) => {
    if (item && frames[index]) {
      item.render(frames[index]);
      frames[index] = null;
    }
  })
  timeout = setTimeout(() => {
    sendMessage();
  }, 1000 / FPS);
}

function transformData(buffer) {
  // console.time('渲染计时器')
  // console.time('解析计时器')
  let unit8Array = new Uint8Array(buffer);
  // console.log(222, unit8Array)
  let message = AwesomeMessage.decode(unit8Array);
  let object = AwesomeMessage.toObject(message);
  // console.log(3333, object);

  let imageBlob;
  let imageWidth;
  let imageHeight;
  let performance = [];
  let smartMsgData = [];
  if (object) {
    // 性能数据
    if (object['StatisticsMsg_'] &&
        object['StatisticsMsg_']['attributes_'] &&
        object['StatisticsMsg_']['attributes_'].length
    ) {
      performance = object['StatisticsMsg_']['attributes_']
    }
    // 图片
    if (object['img_'] &&
        object['img_']['buf_'] &&
        object['img_']['buf_'].length
    ) {
      imageBlob = new Blob([object['img_']['buf_']], { type: 'image/jpeg' });
      imageWidth = object['img_']['width_'] || 1920
      imageHeight = object['img_']['height_'] || 1080
    }
    // 智能数据
    if (object['smartMsg_'] &&
        object['smartMsg_']['targets_'] &&
        object['smartMsg_']['targets_'].length
    ) {
      object['smartMsg_']['targets_'].map(item => {
        if (item) {
          let obj = {
            id: item['trackId_'],
            boxes: [],
            attributes: { attributes: [], type: item['type_'] },
            fall: { fallShow: false },
            points: [],
            segmentation: [],
          }
          let labelStart = 0;
          let labelCount = 20;
          // 检测框
          if (messageShowSelect.boxes && item['boxes_'] && item['boxes_'].length ) {
            item['boxes_'].map((val, ind) => {
              let boxs = transformBoxes(val)
              if (boxs) {
                obj.boxes.push({ type: val['type_'] || '', p1: boxs.box1, p2: boxs.box2 });
                if (ind === 0) {
                  obj.attributes.box = { p1: boxs.box1, p2: boxs.box2 }
                  obj.fall.box = { p1: boxs.box1, p2: boxs.box2 }
                }
                boxs = null;
              }
              
              if (messageShowSelect.scoreShow && ind === 0) {
                obj.attributes.score = val.score
              }
            })
            obj.boxes = !messageShowSelect.handBox && obj.boxes.length ? obj.boxes.filter(item => item.type !== 'hand') : obj.boxes
          }
          // 关节点
          if (item['points_'] && item['points_'].length ) {
            item['points_'].map(val => {
              if (val['points_'] && val['points_'].length) {
                let bodyType = messageShowSelect.body ? '' : 'body_landmarks'
                let faceType = messageShowSelect.face ? '' : 'face_landmarks'
                if (val['type_'] === 'mask') {
                  // 目标分割
                  if (messageShowSelect.floatMatrixsMask) {
                    obj.segmentation.push({ type: 'target_img', data: val['points_'] })
                  }
                } else if (val['type_'] === 'hand_landmarks') {
                  if (messageShowSelect.handMarks) {
                    obj.points.push({ type: val['type_'], skeletonPoints: transformPoints(val['points_'])})
                  }
                } else if (val['type_'] === 'corner') {
                  if (messageShowSelect.corner) {
                    obj.points.push({ type: val['type_'], skeletonPoints: transformPoints(val['points_'])})
                  }
                }  else if (val['type_'] === 'lmk_106pts') {
                  if (messageShowSelect.face) {
                    obj.points.push({ type: val['type_'], diameterSize: 2, skeletonPoints: transformPoints(val['points_'])})
                  }
                } else if (val['type_'] === 'parking') {
                  if (messageShowSelect.boxes) {
                    obj.points.push({ type: val['type_'], skeletonPoints: transformPoints(val['points_'])})
                  }
                } else if (val['type_'] !== bodyType && val['type_'] !== faceType) {
                  let skeletonPoints = [];
                  val['points_'].map((val, index) => {
                    let key = Config.skeletonKey[index];
                    skeletonPoints[key] = {
                      x: val['x_'],
                      y: val['y_'],
                      score: val['score_'] || 0
                    };
                  });
                  obj.points.push({ type: val['type_'], skeletonPoints })
                }
              }
            })
          }
          // 属性
          if (item['attributes_'] && item['attributes_'].length) {
            item['attributes_'].map(val => {
              // 分割的颜色参数
              labelStart = val['type_'] === "segmentation_label_start" ? val['value_'] : 0
              labelCount = val['type_'] === "segmentation_label_count" ? val['value_'] : 20
              // 摔倒
              if (val['type_'] === 'fall' && val['value_'] === 1) {
                obj.fall.fallShow = true;
                obj.fall.attributes = {
                  type: val['type_'],
                  value: val['value_'],
                  score: messageShowSelect.scoreShow ? val['score_'] : undefined
                };
              } else if (messageShowSelect.attributes && val['valueString_']) {
                obj.attributes.attributes.push({
                  type: val['type_'],
                  value: val['valueString_'],
                  score: messageShowSelect.scoreShow ? val['score_'] : undefined
                })
              }
            })
          }
          // 车
          if (item['subTargets_'] && item['subTargets_'].length) {
            item['subTargets_'].map(val => {
              if (messageShowSelect.boxes && val['boxes_'] && val['boxes_'].length) {
                let boxs = transformBoxes(val['boxes_'][0])
                if (boxs) {
                  obj.boxes.push({ p1: boxs.box1, p2: boxs.box2 })
                  boxs = null;
                }
              }
              // 车牌框数据
              if (messageShowSelect.attributes && val['attributes_'] && val['attributes_'].length) {
                val['attributes_'].map(obj => {
                  if (obj['valueString_']) {
                    obj.attributes.attributes.push({
                      type: obj['type_'],
                      value: obj['valueString_'],
                      score: messageShowSelect.scoreShow ? obj['score_'] : undefined
                    })
                  }
                })
              }
            })
          }
          // 全图分割 segmentation parking_mask
          if (messageShowSelect.floatMatrixs &&
              item['floatMatrixs_'] &&
              item['floatMatrixs_'].length
          ) {
            let color = [];
            let step = 255 * 3 / labelCount;
            for (let i = 0; i < labelCount; ++i) {
              let R = (labelStart / 3 * 3) % 256;
              let G = (labelStart / 3 * 2) % 256;
              let B = (labelStart / 3) % 256;
              color.push([R, G, B]);
              labelStart += step;
            }
            if (color.length) {
              let floatdata = []
              item['floatMatrixs_'][0]['arrays_'].map(values => {
                values['value_'].map(index => {
                  let colors = color[Math.trunc(index)]
                  floatdata.push(colors[0], colors[1], colors[2], 155)
                })
              })
              obj.segmentation.push({
                type: 'full_img',
                w: item['floatMatrixs_'][0]['arrays_'][0]['value_'].length,
                h: item['floatMatrixs_'][0]['arrays_'].length,
                data: floatdata
              })
            }
          }
          smartMsgData.push(obj);
          obj = null;
        }
        return null;
      })
    }
  }
  // console.timeEnd('解析计时器')
  return {
    imageBlob,
    imageWidth,
    imageHeight,
    performance,
    smartMsgData
  };
};

function transformBoxes(box) {
  if (box &&
      box['topLeft_']['x_'] &&
      box['topLeft_']['y_'] &&
      box['bottomRight_']['x_'] &&
      box['bottomRight_']['y_']
  ) {
    let box1 = {
      x: box['topLeft_']['x_'],
      y: box['topLeft_']['y_']
    };
    let box2 = {
      x: box['bottomRight_']['x_'],
      y: box['bottomRight_']['y_']
    };
    return { box1, box2 };
  }
};

function transformPoints(points) {
  let skeletonPoints = [];
  points.map((val) => {
    skeletonPoints.push({
      x: val['x_'],
      y: val['y_'],
      score: val['score_'] || 0
    });
  });
  return skeletonPoints;
};
