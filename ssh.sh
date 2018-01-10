#!/usr/bin/env bash
tar -czf - recording.wav | ssh root@wechat.szfda.cn "tar -xzf - -C ~/answer/auto-answer; cd answer/auto-answer; ./recognize.sh"
