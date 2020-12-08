# 标志牌分类

1. 标志牌独立分支
- 首先用大图进行检测，检测框作为分类模型输入，输入roi大小: 64x64
- 输出55类分类结果

# 训练

1. pipeline test

用于检查环境，快速debug等
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
    --config configs/2pe_traffic_sign_cls/multitask.py \
    --pipeline-test
```

2. 训练
```bash
bash tools/quanti_training_2step_fusion_bn.sh \
    --config configs/2pe_traffic_sign_cls/multitask.py \
```
