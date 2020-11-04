# IHBC 光源分类模型

* 该任务以大图中的roi图像作为输入，输出为roi中光源的类别。
* ```light_source_category_classification.py``` 8类光源分类


## Quantization Training, fusion bn

- pipeline test
  - 用验证集跑通整个pipeline，加快debug速度
```bash
sh tools/quanti_training_2step_fusion_bn.sh  --config configs/2pe_ihbc/multitask.py --pipeline-test
```

- local run
  - 用训练集正式训练
```bash
sh tools/quanti_training_2step_fusion_bn.sh  --config configs/2pe_ihbc/multitask.py
```
