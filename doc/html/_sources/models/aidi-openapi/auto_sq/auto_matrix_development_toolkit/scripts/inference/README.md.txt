# AdasInference

这是一个基于mxnet gluon接口的Horizon AUTO感知模型预测框架。在`python3.6`及以上测试通过。

## 4pe预测
```
# resize模型, outputs will be ./results/4pe_resize
python3 scripts/tools/run_infer_one_image.py --img scripts/data/demo.jpg  --config scripts/models/4pe_resize/4pe_resize_model.yaml

# crop模型, outputs will be ./results/4pe_crop
python3 scripts/tools/run_infer_one_image.py --img scripts/data/demo.jpg  --config scripts/models/4pe_crop/4pe_crop_model.yaml
```

## merge结果
```
# outputs will be ./merge_results/merge_4pe_resize_4pe_crop
python3 scripts/tools/run_merge.py --main-dir results/4pe_resize --aux-dir results/4pe_crop --img scripts/data/demo.jpg
```
