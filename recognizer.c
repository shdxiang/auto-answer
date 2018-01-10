#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msp_cmn.h"
#include "msp_errors.h"
#include "qisr.h"

// 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
#define READ_SIZE 6400
#define USLEEP_TIME 1000
#define MAX_WRITE_COUNT 100

// 登录参数，appid与msc库绑定,请勿随意改动
static const char *g_login_params = "appid = 5a548c0f, work_dir = .";

/*
 * sub:				请求业务类型
 * domain:			领域
 * language:			语言
 * accent:			方言
 * sample_rate:		音频采样率
 * result_type:		识别结果格式
 * result_encoding:	结果编码格式
 *
 */
static const char *g_session_params =
    "sub = iat, domain = iat, language = zh_cn, accent = mandarin, "
    "sample_rate = 16000, result_type = plain, result_encoding = utf8";

static int g_exit = 0;

static void sigint_handler(int sig) { g_exit = 1; }
static void sigterm_handler(int sig) { g_exit = 1; }

static int read_data(char *buf, int *len) {
  int ret = 0;

  ret = read(STDIN_FILENO, buf + *len, READ_SIZE - *len);
  if (ret > 0) {
    *len += ret;
  } else if (ret < 0) {
    if (errno != EAGAIN && errno != EINTR) {
      return -1;
    }
  }
  return ret;
}

static void run_iat() {
  const char *session_id = NULL;
  char pcm_data[READ_SIZE] = {0};
  unsigned int total_len = 0;
  int aud_stat = MSP_AUDIO_SAMPLE_CONTINUE; //音频状态
  int ep_stat = MSP_EP_LOOKING_FOR_SPEECH;  //端点检测
  int rec_stat = MSP_REC_STATUS_SUCCESS;    //识别状态
  int errcode = MSP_SUCCESS;
  int ret = 0;
  int read_size = 0;
  int write_count = 0;
  const char *result = NULL;
  int restart = 1;

  //听写不需要语法，第一个参数为NULL
  session_id = QISRSessionBegin(NULL, g_session_params, &errcode);
  if (MSP_SUCCESS != errcode) {
    printf("QISRSessionBegin failed, error code: %d\n", errcode);
    goto iat_exit;
  }

  while (!g_exit) {
    if (restart) {
      write_count = 0;
      aud_stat = MSP_AUDIO_SAMPLE_FIRST;
      restart = 0;
    } else {
      aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
    }

    //读取音频文件内容
    ret = read_data(pcm_data, &read_size);
    if (ret != 0) {
      printf("read error: %d\n", errno);
      goto iat_exit;
    }

    if (read_size == READ_SIZE) {
      ret = QISRAudioWrite(session_id, pcm_data, READ_SIZE, aud_stat, &ep_stat,
                           &rec_stat);
      if (MSP_SUCCESS != ret) {
        printf("QISRAudioWrite failed, error code: %d\n", ret);
        goto iat_exit;
      }
      ++write_count;
    }

    //已经有部分听写结果
    if (MSP_REC_STATUS_SUCCESS == rec_stat) {
      result = QISRGetResult(session_id, &rec_stat, 0, &errcode);
      if (MSP_SUCCESS != errcode) {
        printf("QISRGetResult failed, error code: %d\n", errcode);
        goto iat_exit;
      }
      if (NULL != result) {
        printf(result)
      }
    }

    if (MSP_EP_AFTER_SPEECH == ep_stat) {
      printf("MSP_EP_AFTER_SPEECH\n");
      break;
    }

    if (write_count >= MAX_WRITE_COUNT) {
      errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST,
                               &ep_stat, &rec_stat);
      if (MSP_SUCCESS != errcode) {
        printf("QISRAudioWrite failed, error code: %d\n", errcode);
        goto iat_exit;
      }

      while (!g_exit && MSP_REC_STATUS_COMPLETE != rec_stat) {
        result = QISRGetResult(session_id, &rec_stat, 0, &errcode);
        if (MSP_SUCCESS != errcode) {
          printf("QISRGetResult failed, error code: %d\n", errcode);
          goto iat_exit;
        }
        if (NULL != result) {
          printf(result)
        }
        usleep(USLEEP_TIME);
      }
      restart = 1;
    }
    usleep(USLEEP_TIME);
  }

iat_exit:

  QISRSessionEnd(session_id, "normal");
}

int main(int argc, char *argv[]) {
  int ret = MSP_SUCCESS;

  signal(SIGINT, sigint_handler);
  signal(SIGTERM, sigterm_handler);

  /* 用户登录 */
  //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
  ret = MSPLogin(NULL, NULL, g_login_params);
  if (MSP_SUCCESS != ret) {
    printf("MSPLogin failed , Error code %d.\n", ret);
    goto exit; //登录失败，退出登录
  }

  run_iat();

exit:
  MSPLogout(); //退出登录

  return 0;
}
