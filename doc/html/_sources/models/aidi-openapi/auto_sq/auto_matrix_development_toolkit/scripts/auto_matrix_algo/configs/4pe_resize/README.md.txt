# resize多任务
- 包含4个检测分支
  - `det_cyclist_light.py` 骑车人、红绿灯检测
  - `det_person.py` 行人检测
  - `det_rear.py` 车头车尾分支
  - `det_vehicle_sign.py` 全车、标志牌检测
- 包含2个分割分支
  - `seg_default.py` 12类语义分割
  - `seg_lane.py` 4类车道线分割


# Quantization Training, 2 steps fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
    --config configs/4pe_resize/multitask.py \
    --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
  --config configs/4pe_resize/multitask.py \
```

# 训练简介
- 量化训练
  - 训练共step 0, 1, 2, 3四步，对应`multitask.py solver`的index，包含1、2两步吸收BN训练（吸收后op减少，计算量降低）
  - step 0是with bn training
  - step 1吸收了shared backbone的BN层
  - step 2吸收了各个单任务的head的BN层
  - step 3将step 2模型转换为芯片支持的`kIntInference`模型，不做训练，模型输出与step 2相等

- 多任务训练
  - 多任务同名参数共享，实现节省算力：每个单任务都有自己的symbol，我们通过`get_unshared_and_shared_params()`
    将各单任务的同名params进行共享

- config结构
  - 所有单任务通用的配置放在`utils.py`
  - 单任务特有的配置放在各自的py文件
  - `solver, env, fusion_bn_patterns`等训练配置放在`multitask.py`

- `model`和`export_model`的区别
  - model只用于训练，export_model用于GPU预测、hbdk编译后在芯片上运行
  - model包含backbone, head, loss（不能用于预测）等，export_model包含backbone, head，不含loss
  - step 3的export_model包含`DetectionPostProcessing_X2`、`channel_argmax`等在芯片上部署所需的post processing op，
    其余的model、export_model都不含这些op
  - model的checkpoint：`with-bn-init.params, with-bn-0001.params, with-bn-last-0000.params ...`
    - init是训练开始时的checkpoint，0001是训练中，last是训练结束
    - 目前model的checkpoint不包含symbol.json
  - export_model的checkpoint：`with-bn-export-last-0000.params, with-bn-export-last-symbol.json ...`

- 用于GPU预测的模型
  - step 2的最后一个checkpoint`fusion-bn-c2-export-last-0000.params, fusion-bn-c2-export-last-symbol.json`

- 用于hbdk编译，然后在芯片上部署的模型
  - step 3的最后一个checkpoint`int-inference-export-last-0000.params, int-inference-export-last-symbol.json`

- crop和resize模型的区别有三点
  1. 模型输入尺寸，以训练阶段为例
      - crop：  `720x1280原图` --random crop-> `448x1216`
      - resize：`720x1280原图` --等比例resize-> `540x960` --random crop-> `512x960`
      - crop关注远处小目标，resize关注近处大目标，crop + resize方案比将原图直接输入模型的方案的计算量更小
  2. 模型容量大小
      - crop用的是`vargnet_v2_g8g4_0_5x`，resize用的是`vargnet_v2_g8g8_0_5x`，upscale_group_base分别是4、8
      - export_model的输入尺寸，crop比resize大，当前算力条件下，crop只支持upscale_group_base 4，可根据算力调整
  3. resize比crop多了语义分割任务`seg_default.py`

- resize模型输入尺寸
  - 设置`resize_hw = [540, 1140], resize_type = 'min_max_pairs'`，实现等比例resize
  - model（训练）的输入尺寸，训练集可包含多种尺寸，如下列出常见尺寸从原图到模型输入所经历的变化
    - `720x1280`  --等比例resize-> `540x960`  --random crop-> `512x960`
    - `940x1824`  --等比例resize-> `540x1048` --random crop-> `512x960`
    - `1080x1920` --等比例resize-> `540x960`  --random crop-> `512x960`
    - `1080x2280` --等比例resize-> `540x1140` --random crop-> `512x960`
  - export_mode（预测、部署）的输入尺寸，如下尺寸选一种，例子用的是`940x1824`输入
    - `940x1824`  --pyramid resize 1/2--> `470x912` --去掉上方22像素--> `448x912` --右边padding 48像素--> `448x960`
    - `1080x1920` --pyramid resize 1/2--> `540x960` --去掉上方28像素--> `512x960`
    - `1080x2280` --pyramid resize 1/2--> `540x1140` --去掉上方28像素--> `512x1140`

# 其它
- 把多任务改成分割、车道线分割单任务
  - 需要把`multitask.py`里的`int_inference_export_input_shape`的`im_hw`删掉，
    检测才需要它，不删会报input layout错误
- 适当增大`load_threads, threads, shuffle_chunck_size`可以加快io速度，进而加快训练速度
- `utils.py default_ctx`设置了默认的GPU卡数，可通过`train.py --ctx 0,1`动态调整GPU卡数
