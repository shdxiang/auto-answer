Debug use:

```
export LD_LIBRARY_PATH=$(pwd)/libs:${LD_LIBRARY_PATH}
```

Recording:

```
sox -d -p | sox -p -b 16 -r 16000 data/audio.wav remix -
```

Docker build and run:

```
docker build -t auto-answer .
docker run -td -v `pwd`/data:/root/data --name auto-answer auto-answer
```

Docker execute:

```
docker exec auto-answer recognizer /root/data/audio.wav
```

Auto-reload browser:

```
browser-sync start --server 'web' --files 'web'
```
