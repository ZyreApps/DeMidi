FROM ubuntu:trusty
MAINTAINER Benjamin Henrion <zoobab@gmail.com>

RUN DEBIAN_FRONTEND=noninteractive apt-get update -y -q
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y -q --force-yes uuid-dev build-essential git-core libtool autotools-dev autoconf automake pkg-config unzip libkrb5-dev cmake libasound2-dev

RUN useradd -d /home/zmq -m -s /bin/bash zmq
RUN echo "zmq ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/zmq
RUN chmod 0440 /etc/sudoers.d/zmq

USER zmq

WORKDIR /home/zmq
RUN git clone https://github.com/zeromq/libzmq.git
WORKDIR /home/zmq/libzmq
RUN ./autogen.sh
RUN ./configure --without-docs
RUN make
RUN sudo make install
RUN sudo ldconfig

WORKDIR /home/zmq
RUN git clone https://github.com/zeromq/czmq.git
WORKDIR /home/zmq/czmq
RUN ./autogen.sh
RUN ./configure --without-docs
RUN make
RUN sudo make install
RUN sudo ldconfig

WORKDIR /home/zmq
RUN git clone https://github.com/zeromq/zyre.git
WORKDIR /home/zmq/zyre
RUN ./autogen.sh
RUN ./configure --without-docs
RUN make
RUN sudo make install
RUN sudo ldconfig

WORKDIR /home/zmq
RUN git clone https://github.com/ZyreApps/DeMidi.git
WORKDIR /home/zmq/DeMidi
RUN ./autogen.sh
RUN ./configure --without-docs
RUN make
RUN sudo make install
RUN sudo ldconfig

RUN which midigrab || echo "midigrab not found" && exit 2
RUN which midiplay || echo "midiplay not found" && exit 2
