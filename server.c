#include "common.h"
#include "server.h"

/*
 * 클라이언트로부터 받은 메세지를 다시 전송하는 함수.
 */
int echo_message(FILE* log_fd, int fd, struct sockaddr_in client_addr)
{
    enum errnum err;
    chat_message_t* one_msg;      // client에서 받은 message 구조체
    char* client_ip;
    int client_port;

    if (log_fd == NULL) {
        err = E_PARAM;
        return err;
    }

    client_ip = inet_ntoa(client_addr.sin_addr);
    client_port = ntohs(client_addr.sin_port);

    // allocation memory for struct chat_message.
    one_msg = init_chat_message();
    if (one_msg == NULL) {
        err = E_INITMSG;
        return err;
    }

    err = recv_message(fd, one_msg);
    if (err != 0) {
        free_chat_message(one_msg);
        err = E_RECVMSG;
        return err;
    }

    // print a message onto the console and the log file.
    fprintf(log_fd, "< %s_%d >: %s",
            client_ip, client_port, one_msg->msg);
    printf("< %s_%d >: %s",
            client_ip, client_port, one_msg->msg);

    err = send_message(fd, one_msg);
    if (err < 0) {
        free_chat_message(one_msg);
        err = E_SENDMSG;
        return err;
    }

    // quit check.
    if (strncmp((const char*)one_msg->msg, "quit\n",
                one_msg->msg_len) == 0) {

        free_chat_message(one_msg);
        err = E_CONNCLS;
        return err;
    }

    free_chat_message(one_msg);
    return OK;
}


/*
 * Single process 에서 동작하는 Multi client들을 처리하는 서버.
 */
#define MYPORT          1234    // 내 포트는 1234로 정함.
#define BACKLOG         100
int run_server(void)
{
    enum errnum err;
    int server_fd;             // server socket file descriptor
    int client_fd;             // clinet socket file descriptor
    struct sockaddr_in my_addr;    // 내 주소.
    struct sockaddr_in client_addr; // client의 주소.
    unsigned int sin_size;

    fd_set read_set, set;
    int fd_hwm = 0;     // 관찰 범위 hight water mark
    int fd_num = 0;     // 변경된 fd 개수.
    int cur_fd;

    FILE* log_fd;
    int echo_comm_ret;

    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        err = E_SOCKFAIL;
        return err;
    }

    config_server_addr(&my_addr);

    if(bind(server_fd,(struct sockaddr *)&my_addr,
                sizeof(struct sockaddr)) == -1) {

        perror("bind");
        err = E_BINDFAIL;
        return err;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        err = E_LISTENFAIL;
        return err;
    }

    set_fd_nonblock_flag(server_fd);

    FD_ZERO(&set);              // set 초기화.
    FD_SET(server_fd, &set);    // server socket 등록.
    fd_hwm = server_fd;         // 관찰 범위에 server_fd 추가.

    while (1) {
        read_set = set;         // read set 갱신
        fd_num = select(fd_hwm + 1, &read_set, NULL, NULL, NULL);
        if (fd_num < 0) {
            perror("select");
            err = E_SELECTFAIL;
            close(server_fd);
            return err;
        }

        for (cur_fd = 0; cur_fd <= fd_hwm; ++cur_fd) {
            if (FD_ISSET(cur_fd, &read_set)){
                if (cur_fd == server_fd) {
                    client_fd = accept(server_fd,
                            (struct sockaddr *)&client_addr, &sin_size);

                    if (client_fd == -1) {  // nonblock error
                        if (errno == -EAGAIN || errno == -EWOULDBLOCK) {
                            continue;
                        }
                        perror("accept");
                        err = E_ACCEPTFAIL;
                        close(server_fd);
                        return err;
                    }

                    FD_SET(client_fd, &set);
                    if (client_fd > fd_hwm) {
                        fd_hwm = client_fd;
                    }
                }
                else {      // client
                    log_fd = fopen("./server.log", "a+");
                    if (log_fd == NULL) {
                        perror("logfile");
                        err = E_LOGFAIL;
                        close(cur_fd);
                        return err;
                    }

                    echo_comm_ret = echo_message(log_fd, cur_fd, client_addr);
                    fclose(log_fd);
                    if (echo_comm_ret < 0) {
                        if (echo_comm_ret == E_CONNCLS) {
                            FD_CLR(cur_fd, &set);   // connection closed.
                            if (fd_hwm == cur_fd) {
                                fd_hwm--;
                            }
                            close(cur_fd);
                            continue;
                        }
                        err = E_ECHO_FAIL;
                        close(cur_fd);
                        return err;
                    }
                }
            }
        }//for end.
    }// while end.
    close(server_fd);
    return OK;
}

void config_server_addr(struct sockaddr_in* my_addr)
{
    // config my info
    memset(my_addr, 0, sizeof(struct sockaddr_in));
    my_addr->sin_family = AF_INET;         // host byte order
    my_addr->sin_port = htons(MYPORT);     // short, network byte order
    my_addr->sin_addr.s_addr = INADDR_ANY; // auto-fill with my IP
}
