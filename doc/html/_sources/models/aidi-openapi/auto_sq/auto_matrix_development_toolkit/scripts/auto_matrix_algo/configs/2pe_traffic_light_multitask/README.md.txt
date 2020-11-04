# 基于灯箱检测框的交通灯分类多任务
- 包含3个分类分支
  - `classification_traffic_light_color.py`  交通灯颜色分类
  - `classification_traffic_light_category.py` 交通灯类别分类
  - `classification_traffic_light_fp.py` 交通灯误检分类

# Quantization Training, fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
sh tools/quanti_training.sh  --config configs/2pe_traffic_light_multitask/multitask.py --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
sh tools/quanti_training.sh  --config configs/2pe_traffic_light_multitask/multitask.py
```

# 训练简介
- 更多可参考`configs/4pe_resize/README.md`
