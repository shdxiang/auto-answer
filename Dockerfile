FROM ubuntu:16.04

ADD ./libs/libmsc.so /lib/libmsc.so
ADD ./bin/recognizer /usr/bin/recognizer

VOLUME ["~/data"]
