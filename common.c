#include "common.h"

err_msg_t get_err_msg[] = {
    {E_PARAM, "parameter error"},
    {E_ALLOC, "memory allocation error"},
    {E_SEND, "send function error"},
    {E_SENDMSG, "send message fuction error"},
    {E_RECV, "recv function error"},
    {E_RECVMSG, "recv message fuction error"},
    {E_PEER_LOST, "Peer connection done"},
    {E_LENGTH_ZERO, "receive length zero"},
    {E_ECHO_INIT, "echo init error"},
    {E_CONNCLS, "connection closed"},
    {E_SOCKFAIL, "socket open error in echo"},
    {E_BINDFAIL, "socket bind error in echo"},
    {E_LISTENFAIL, "socket listen error in echo"},
    {E_SELECTFAIL, "select function error in echo"},
    {E_ACCEPTFAIL, "accept error in echo"},
    {E_LOGFAIL, "open log file error"},
    {E_ECHO_FAIL, "echo_message error"},
    {E_INITMSG, "message init error"},
    {E_INVALADDR, "invalid address"}, // in client.
    {E_CONNFAIL, "connection fail"},
    {E_GETMSGFAIL, "get message function error"}
};

char* find_err_msg(int errcode)
{
    int i;
    err_msg_t e;
    int err;

    // errcode가 음수이기에 양으로 만든다.
    err = errcode * -1;

    if (err >= MAX_NUM_ERR) {
        printf("error code over\n");
        return NULL;
    }

    for (i = 0; i < sizeof(get_err_msg)/sizeof(err_msg_t); i++) {
        e = get_err_msg[i];
        if (e.errnum == errcode) {
            return e.msg;
        }
    }

    return NULL;
}

/*
 * 구조체 chat_message_t의 내용을 전송한다.
 * return
 *  정상
 *      OK : 0
 *  error
 *      E_PARAM  : 인자값 에러
 *      E_ALLOC  : 메모리 할당 에러.
 *      E_SEND   : send 함수 에러.
 */
int send_message(int fd, chat_message_t* m)
{
    int err;
    size_t total_msg_len;
    uint32_t net_msg_len;   // network에서 사용될 uint32_t
    char* send_buf;         // 구조체 client_msg를 client에 보내기위한 버퍼

    if (m == NULL) {
        return E_PARAM;
    }

    // send_buf의 size = 4bytes(uint32_t) + message(chars의 길이)
    total_msg_len = sizeof(uint32_t) + m->msg_len;
    send_buf = (char*)malloc(total_msg_len);
    if (send_buf == NULL) {
        return E_ALLOC;
    }
    memset(send_buf, 0x00, total_msg_len);

    // hton uint32_t endian 변경.
    net_msg_len = htonl(m->msg_len);

    // memcpy to buf
    memcpy(send_buf, &net_msg_len, sizeof(uint32_t));    // 4bytes 만큼 memcpy
    memcpy(send_buf + sizeof(uint32_t), m->msg, m->msg_len);

    err = send(fd, send_buf, total_msg_len, 0);
    if (err == -1) {
        perror("send");
        free(send_buf);
        return E_SEND;
    }

    free(send_buf);
    return OK;
}


/*
 * 받은 데이터를 client_msg구조체에 넣고 리턴하는 함수.
 * return
 *  정상
 *      OK : 0
 *  error
 *      E_PARAM  : 인자값 에러
 *      E_ALLOC  : 메모리 할당 에러.
 *      E_SEND   : send 함수 에러.
 *      E_LENGTH_ZERO : 받은 길이가 0인 에러.
 *      E_PEER_LOST : Client의 Connection이 shutdown될 때.
 */
#define RECV_BUF_SIZE   255
#define UINT32_SIZE     4
#define RECV_ERROR      -1
int recv_message(int fd, chat_message_t* m)
{
    ssize_t numbytes;
    ssize_t msg_sum = 0;
    char recv_buf[RECV_BUF_SIZE] = {0,};

    if (m == NULL) {
        return E_PARAM;
    }

    while ((numbytes = recv(fd, recv_buf, RECV_BUF_SIZE, 0)) > 0) {
        if (msg_sum == 0) {
            memcpy((void*)m, recv_buf, UINT32_SIZE);
            m->msg_len = ntohl(m->msg_len); // get length
            if (m->msg_len == 0) {
                return E_LENGTH_ZERO;
            }

            m->msg = (char*)malloc(sizeof(char) * m->msg_len);
            if (m->msg == NULL) {
                return E_ALLOC;
            }

            msg_sum = numbytes - (ssize_t)UINT32_SIZE;
            memcpy(m->msg, recv_buf + UINT32_SIZE, msg_sum);
        }
        else {
            memcpy(m->msg + msg_sum, recv_buf, numbytes);
            msg_sum += numbytes;
        }

        if (msg_sum == m->msg_len) {
            return OK;
        }
    }
    if (numbytes == RECV_ERROR) {
        if (errno == -EAGAIN || errno == -EWOULDBLOCK) {
            return OK;   // nonblock 시에는 정상 리턴.
        }
        else {
            return E_RECV;
        }
    }
    else if (numbytes == 0) {
        return E_PEER_LOST;
    }

    return OK;
}


chat_message_t* init_chat_message(void)
{
    chat_message_t* temp;

    temp = (chat_message_t*)malloc(sizeof(chat_message_t));
    if (temp == NULL) {
        return NULL;
    }
    temp->msg_len = -1;
    temp->msg = NULL;

    return temp;
}

int free_chat_message(chat_message_t* m)
{
    if (m == NULL) {
        return -1;
    }

    if (m->msg != NULL) {
        free(m->msg);
    }

    free(m);
    return OK;
}

/* 안에 내용물은 모두 free 또는 초기화하고 구조체는 남기고 리턴*/
int cleanup_chat_message(chat_message_t* m)
{
    if (m == NULL) {
        return -1;
    }

    m->msg_len = -1;
    if (m->msg != NULL) {
        free(m->msg);
        m->msg = NULL;
    }
    m->msg = NULL;
    return OK;
}

void set_fd_nonblock_flag(int fd)
{
    int fd_flags;

    fd_flags = fcntl(fd, F_GETFL);
    fd_flags |= O_NONBLOCK;
    fd_flags = fcntl(fd, F_SETFL, fd_flags);
}
