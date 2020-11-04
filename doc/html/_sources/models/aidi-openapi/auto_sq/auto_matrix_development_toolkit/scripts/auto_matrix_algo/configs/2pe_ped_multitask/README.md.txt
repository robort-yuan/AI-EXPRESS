# 车头车尾2PE多任务

# Quantization Training, fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
sh tools/quanti_training_2step_fusion_bn.sh  --config configs/2pe_ped_multitask/multitask.py --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
sh tools/quanti_training_2step_fusion_bn.sh  --config configs/2pe_ped_multitask/multitask.py
```
