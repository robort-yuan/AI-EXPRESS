## Prepare data

We use the adas_mini_dataset for examples.
Data should be organize in the following way

```bash
data/
    2pe_traffic_sign_classification/
        train/
        val/
    4pe_traffic_sign/
        train/
        val/
```

## Output

After packing data, output data should looks like

```bash
data/
    rec/
        2pe_traffic_sign_classification/
            train.rec
            train.json
            val.rec
            val.json
        4pe_traffic_sign/
            train.rec
            train.json
            val.rec
            val.json
```

## How to run examples
```bash
bash tools/pipeline.sh --config configs/2pe_traffic_sign_classification/train.py
```
