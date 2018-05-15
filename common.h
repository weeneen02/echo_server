#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <assert.h>

#define OK          0
#define MAX_NUM_ERR 52

enum errnum {
    E_PARAM = -1,       // 공통적인 에러.
    E_ALLOC = -2,
    E_SEND  = -13,      // send_message 함수 에러
    E_SENDMSG = -14,
    E_RECV  = -23,      // recv_message 함수 에러
    E_RECVMSG = -24,
    E_PEER_LOST = -25,
    E_LENGTH_ZERO = -26,
    E_ECHO_INIT = -31,  // echo_message 함수 에러
    E_CONNCLS = -32,
    E_SOCKFAIL = -41,   // run_server 함수 에러
    E_BINDFAIL = -42,
    E_LISTENFAIL = -43,
    E_SELECTFAIL = -44,
    E_ACCEPTFAIL = -45,
    E_LOGFAIL = -46,
    E_ECHO_FAIL = -47,
    E_INITMSG = -48,
    E_INVALADDR = -50,  // client의 main 함수 에러.
    E_CONNFAIL = -51,
    E_GETMSGFAIL = -52,
};

typedef struct error_msg {
    int errnum;
    char* msg;
} err_msg_t;

typedef struct chat_message {
    uint32_t msg_len;
    char* msg;
} chat_message_t;

chat_message_t* init_chat_message(void);
int cleanup_chat_message(chat_message_t* m);
int free_chat_message(chat_message_t* m);

int recv_message(int fd, chat_message_t* msg);
int send_message(int fd, chat_message_t* msg);

void set_fd_nonblock_flag(int fd);
char* find_err_msg(int errcode);
#endif
