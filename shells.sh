#!/usr/bin/env bash
tar -czf - recording.wav | ssh root@wechat.szfda.cn "tar -xzf - -C ~/answer/auto-answer; cd answer/auto-answer; ./recognize.sh"
docker run -td --name auto-answer auto-answer
export LD_LIBRARY_PATH=$(pwd)/libs/x64:${LD_LIBRARY_PATH}
