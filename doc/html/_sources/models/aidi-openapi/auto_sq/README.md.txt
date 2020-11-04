# 艾迪平台使用流程介绍

## 流程概述

- 数据打包和上传
- 提交训练任务
- 模型保存
- 模型预测
- 模型评测
- 模型发版与编译

其中，模型保存、模型预测、模型评测和模型发版与编译在二期交付

## 操作步骤

### Step1 数据打包和上传

将adas_mini数据集，打包成rec格式，然后上传到艾迪平台

- 数据打包
  用于进行数据打包的脚本入口在`auto_matrix_development_toolkit/scripts/auto_matrix_pack_tools`下
  将adas_mini数据集放到data目录下，然后使用`bash tools/pipeline.sh --config configs/4pe_xxx/train.py`命令来进行数据打包，其中`xxx`表示具体的场景，包括
  - cyclist
  - default_parsing
  - lane_parsing
  - pedestrian
  - traffic_light
  - traffic_sign
  - vehicle
  - vehicle_rear
  打包完成后，在`data/rec`目录下会得到各类场景的打包结果文件
  
- 数据上传
  将打包结果文件上传到艾迪平台，在训练时作为输入
  在`data/rec`目录下，使用hitc命令行工具进行上传
  首先使用`hitc init -t your_token`将用户信息添加到命令行配置中，其中your_token为用户的标识信息，可以在艾迪平台-个人中心页面查看到，hitc命令行的默认配置文件路径为`~/.olympus/config.yaml`
  然后使用`hitc dataspace upload -n adasmini_4pe -t trainset -f ./`将当前目录下的所有rec文件上传到艾迪平台-训练数据集中，其中`adasmini_4pe`为训练集名称，`trainset`表示该数据集为训练集
  上传成功后，在艾迪平台`数据管理/训练数据集`中可以看到对应的数据集，点击进入之后可以看到数据集的详细信息

### Step2 提交训练任务

将本地训练代码包提交到GPU集群运行，具体的训练场景为4pe检测任务

- 配置任务信息
  编辑训练任务的相关信息，包括任务名称、本地的代码包文件路径、代码包加密的密码、任务所需要的服务器和GPU数量、任务的启动脚本等必填项，以及任务优先级、使用的Docker镜像、任务的最大运行时间以及挂载的数据集等选填项，具体参见`train_job.yaml`文件
  在此，我们定义训练任务的启动脚本为`auto_matrix_development_toolkit/run.sh`，从中可以看出
  - `tools/quanti_training_2step_fusion_bn.sh`为真正的训练入口
  - `--config ${WORKING_PATH}/scripts/auto_matrix_algo/configs/4pe_resize/multitask.py`为真正的训练任务配置
  - `--ctx 0,1,2,3`表明本次训练使用的GPU卡数为4
  - `--pipeline-test`表示使用快速模式，可以很快的跑完整个训练，但产出的模型精度不高。如果需要保证模型的效果，可以去掉该参数
  同时，在`auto_matrix_development_toolkit/scripts/auto_matrix_algo/configs/4pe_resize/datasets.yaml`，我们定义了训练时读取的各个场景下数据集的路径

- 配置集群信息
  首先在艾迪平台-个人中心页面-集群配置处进行创建APP KEY操作，得到集群队列的Appid和Appkey
  traincli命令行的默认配置文件路径为`~/.hobot/gpucluster.yaml`
  将Appid和Appkey信息填入到traincli配置文件中的`clusters.mycluster1.appid`和`clusters.mycluster1.appkey`字段下
  
- 提交集群训练
  用于进行训练的脚本入口在`auto_matrix_development_toolkit/scripts/auto_matrix_algo`下
  使用`traincli submit -f train_job.yaml`命令，将训练代码包提交到GPU集群运行
  提交成功后，在艾迪平台`任务管理/训练任务`的`运行任务`中标签栏中可以看到创建的训练任务，点击进入之后可以看到训练任务的详细信息
