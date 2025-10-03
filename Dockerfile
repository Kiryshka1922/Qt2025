FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    qt5-default \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libgl1-mesa-dev \
    libxcb-xinerama0 \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN qmake -o Makefile Qt.pro && make -j$(nproc)

CMD ["./image_filter_app"]