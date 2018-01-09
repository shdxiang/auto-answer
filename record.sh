#!/usr/bin/env bash
sox -d -p | sox -p -b 16 -r 16000 recording.wav remix -
