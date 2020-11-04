# Training Scripts

1. Prepare data

data should be organize in the following format

```
data/
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
```

2. Running scripts

Please see the **README.md** in each examples configs, for example,
**configs/4pe_resize/README.md**
