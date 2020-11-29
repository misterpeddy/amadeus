# Data Preparation

This directory contains the preprocessing image. Example build and run:

```bash
docker build -t pedrampejman/amadeus-preprocess .

docker run \
-v /home/peddy/repos/amadeus/preprocessing/:/data \
pedrampejman/amadeus-preprocess:latest \
--input_path=/data/example.wav --output_path=/data/outputs 
```
