#!/usr/bin/env bash
export LD_LIBRARY_PATH=$(pwd)/libs/x64:${LD_LIBRARY_PATH}
./bin/auto-answer recording.wav
