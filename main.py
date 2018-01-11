import logging
import subprocess
from pynput import keyboard

HOT_KEY = keyboard.Key.shift


# sox -d -p | sox -p -b 16 -r 16000 recording.wav remix -

def start_record():
    global g_remix
    global g_record

    logging.debug('recording...')

    args = 'sox -p -b 16 -r 16000 data/audio.wav remix -'.split()
    g_remix = subprocess.Popen(args, stdin=subprocess.PIPE)

    args = 'sox -d -p'.split()
    g_record = subprocess.Popen(args, stdout=g_remix.stdin)


def stop_record():
    global g_remix
    global g_record

    g_record.terminate()
    g_remix.terminate()


# tar -czf - recording.wav | ssh root@wechat.szfda.cn
# "tar -xzf - -C ~/answer/auto-answer; cd answer/auto-answer; ./recognize.sh"


def recognize():
    global g_docker

    logging.debug('recognizing...')

    args = 'docker exec auto-answer recognizer /root/data/audio.wav'.split()
    g_docker = subprocess.Popen(args, stdout=subprocess.PIPE)

    try:
        outs, errs = g_docker.communicate(timeout=5)

        if errs:
            return None
        if outs:
            return outs
    except subprocess.TimeoutExpired:
        g_docker.terminate()
        return None


# curl -L -o web/index.html http://www.baidu.com/s?wd=
def query(data):
    url = 'http://www.baidu.com/s?wd=' + data
    args = 'curl -L -o web/index.html'.split()
    args.append(url[:-1])
    # logging.debug(args)

    g_curl = subprocess.Popen(args)
    g_curl.wait()


def on_key_press(key):
    # logging.debug(key)
    if key == HOT_KEY:
        start_record()


def on_key_release(key):
    # logging.debug(key)
    if key == HOT_KEY:
        stop_record()

        result = recognize()
        if result:
            data = result.decode('utf-8')
            logging.debug(data)
            query(data)


def main():
    logFormat = '%(asctime)s %(filename)s [%(lineno)d][%(levelname)s] %(message)s'
    logging.basicConfig(level=logging.DEBUG, format=logFormat)

    with keyboard.Listener(on_press=on_key_press,
                           on_release=on_key_release) as listener:
        listener.join()


if __name__ == '__main__':
    main()
