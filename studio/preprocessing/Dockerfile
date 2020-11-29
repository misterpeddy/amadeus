# Set base image to Cloud AI Platform's blessed TF GPU image
ARG BASE=gcr.io/deeplearning-platform-release/tf2-gpu.2-2
FROM ${BASE}

# Installs sndfile library for audio file I/O
RUN apt-get update && \
    apt-get install --no-install-recommends -y libsndfile-dev

# Installs DDSP library
RUN pip install --upgrade pip && \
    pip install --upgrade ddsp

# Copy over needed files into the image
WORKDIR /root
COPY preprocess.py preprocess.py

# User flags will be passed to preprocess script
ENTRYPOINT ["python", "preprocess.py"]
