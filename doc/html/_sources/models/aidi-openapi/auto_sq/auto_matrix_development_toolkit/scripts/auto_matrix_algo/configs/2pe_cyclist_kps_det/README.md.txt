# 骑车人角点检测


## Quantization Training, fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
sh tools/quanti_training.sh --config configs/2pe_cyclist_kps_det/cyc_wheel_kpsdet.py --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
sh tools/quanti_training.sh --config configs/2pe_cyclist_kps_det/cyc_wheel_kpsdet.py
```
