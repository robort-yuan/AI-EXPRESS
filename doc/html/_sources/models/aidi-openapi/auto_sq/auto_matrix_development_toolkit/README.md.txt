# Horizon Auto Matrix Development toolkit

## Python Packages

1. gluon_horizon_internal

**gluon_horizon_intenral** extends the **gluon_horizon** python packages,
including features that are used internally.

2. auto_matrix

**auto_matrix** is based on **gluon_horizon** and **gluon_horizon_internal**,
all algorithm of horizon adas perception is implemented in it.

3. AdasInference, crop_roi, pyramid_resizer

**AdasInference** is an inference framework, and **crop_roi** and **pyramid_resizer**
are the dependencies.

## How to run the packing data scripts

1. **scripts/auto_matrix_pack_tools/** contains script to pack adasmini data

2. link to the in adasmini to **scripts/auto_matrix_pack_tools/data**, it should
be organize as

```
scripts/
    auto_matrix_pack_tools/
        data/
            2pe_cyclist_kps/
                train/
                val/
            4pe_cyclist/
                train/
                val/
            ...
```

3. How to run the scripts, please see **scripts/auto_matrix_pack_tools/README.md**

4. After packing adasmini data, all packed data should be in the **scripts/auto_matrix_pack_tools/data/rec** directory,
it should be organize as

```
scripts/
    auto_matrix_pack_tools/
        data/
            rec/
                2pe_cyclist_kps/
                    train.pb_rec
                    train.pb_rec.idx
                    train.anno.pb_rec
                    train.anno.pb_rec.idx
                    val.pb_rec
                    val.pb_rec.idx
                    val.anno.pb_rec
                    val.anno.pb_rec.idx
                4pe_cyclist/
                    train.rec
                    train.json
                    val.rec
                    val.json
                ...
```

## How to run the training scripts

1. Please make sure you have finish packing all the adasmini data

2. **scripts/auto_matrix_algo/** contains all training scripts

3. Link the pack data to **scripts/auto_matrix_algo/**

```bash
cd scripts/auto_matrix_algo
ln -s ../auto_matrix_pack_tools/data/rec data
```

4. How to run the scripts, please see **scripts/auto_matrix_algo/README.md**


## How to run the inference demo

1. Please see **scripts/inference/README.md** for more detail.
