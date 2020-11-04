# crop多任务
- 包含4个检测分支
  - `det_cyclist_light.py` 骑车人、红绿灯检测
  - `det_person.py` 行人检测
  - `det_rear.py` 车头车尾分支
  - `det_vehicle_sign.py` 全车、标志牌检测
- 包含1个分割分支
  - `seg_lane.py` 4类车道线分割

# Quantization Training, 2 steps fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
    --config configs/4pe_crop/multitask.py \
    --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
  --config configs/4pe_crop/multitask.py \
```

# 训练简介
- crop模型输入尺寸
  - model（训练）的输入尺寸，训练集可包含多种尺寸，如下列出常见尺寸从原图到模型输入所经历的变化，样例训练集仅包含`720x1280`图片
    - `720x1280`  --random/center crop-> `448x1216`
    - `940x1824`  --random/center crop-> `448x1216`
    - `1080x1920` --random/center crop-> `448x1216`
    - `1080x2280` --random/center crop-> `448x1216`
    - 如果训练集包含多种尺寸，`max_img_hw`设置为最大尺寸，作用于center crop
  - export_mode（预测、部署）的输入尺寸，如下尺寸选一种，例子用的是`940x1824`输入
    - `940x1824`  --根据相机参数算出消失点，给定crop尺寸--> `448x1280`
- 更多解析见`configs/4pe_resize/README.md`
