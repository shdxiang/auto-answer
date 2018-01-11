import logging
import subprocess
import urllib.parse
from pynput import keyboard


HOT_KEY = keyboard.Key.shift

REMIX = 'sox -p -b 16 -r 16000 data/audio.wav remix -'
RECORD = 'sox -d -p'
RECOGNIZE = 'docker exec auto-answer recognizer /root/data/audio.wav'
QUERY_URL = 'http://www.baidu.com/s?ie=utf-8&wd={}'
QUERY = 'curl -L -o web/index.html'


def start_record():
    global g_remix
    global g_record

    logging.debug('recording...')

    args = REMIX.split()
    g_remix = subprocess.Popen(args, stdin=subprocess.PIPE)

    args = RECORD.split()
    g_record = subprocess.Popen(args, stdout=g_remix.stdin)


def stop_record():
    global g_remix
    global g_record

    g_record.terminate()
    g_remix.terminate()


def recognize():
    logging.debug('recognizing...')

    args = RECOGNIZE.split()
    docker = subprocess.Popen(args, stdout=subprocess.PIPE)

    try:
        outs, errs = docker.communicate(timeout=5)

        if errs:
            return None
        if outs:
            return outs
    except subprocess.TimeoutExpired:
        docker.terminate()
        return None


def query(data):
    transtab = str.maketrans("，。？！", "    ")
    data = data.translate(transtab)

    data = urllib.parse.quote(data.strip())

    logging.debug(data)

    url = QUERY_URL.format(data)
    logging.info(url)

    args = QUERY.split()
    args.append(url)
    logging.debug(args)

    g_curl = subprocess.Popen(args)
    g_curl.wait()


def on_key_press(key):
    logging.debug(key)
    if key == HOT_KEY:
        start_record()


def on_key_release(key):
    logging.debug(key)
    if key == HOT_KEY:
        stop_record()

        result = recognize()
        if result:
            data = result.decode('utf-8')
            logging.info(data)
            query(data)


def main():
    logFormat = '%(asctime)s %(filename)s [%(lineno)d][%(levelname)s] %(message)s'
    logging.basicConfig(level=logging.INFO, format=logFormat)

    with keyboard.Listener(on_press=on_key_press,
                           on_release=on_key_release) as listener:
        listener.join()


if __name__ == '__main__':
    main()
