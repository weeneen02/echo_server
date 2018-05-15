#include "common.h"
#include "client.h"

#define MAXSIZE 4096

int main(int argc, char *argv[])
{
    chat_message_t* my_msg;
	int ret = 0;
	int length = 0;
	int sockfd = 0;
    int	numbytes = 0;
    int err;

	size_t bytes = 0;

	struct in_addr server;
	struct sockaddr_in server_addr;
    int i;

    if (argc != 3) {
        printf("put IP PORT\n");
        return E_PARAM;
    }

	ret = inet_aton(argv[1], &server);
	if (ret == 0) {
		perror("invalid address");
        return E_INVALADDR;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
        return E_SOCKFAIL;
	}
	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr = server;

	ret = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
	if(ret == -1){
		perror("connect");
        return E_CONNFAIL;
	}

    while (1) {
        my_msg = get_message(); // get message from console
        if (my_msg == NULL) {
            printf("%s\n", find_err_msg(E_GETMSGFAIL));
            return E_GETMSGFAIL;
        }

        err = send_message(sockfd, my_msg);
        if (err < 0) {
            // send error
            printf("%s\n", find_err_msg(err));
            break;
        }

        err = cleanup_chat_message(my_msg);

        err = recv_message(sockfd, my_msg);
        if (err < 0) {
            printf("%s\n", find_err_msg(err));
            break;
        }

        printf("server message = ");
        for (i = 0; i < my_msg->msg_len; i++) {
            printf("%c", my_msg->msg[i]);
        }

        // quit check
        if (strncmp(my_msg->msg, "quit\n", my_msg->msg_len) == 0) {
            break;
        }

        free_chat_message(my_msg);
    }

    free_chat_message(my_msg);
    close(sockfd);
    return OK;
}

#define MSG_MAXSIZE     4096
#define INPUT_MSG_SIZE  255
chat_message_t* get_message(void)
{
    chat_message_t* temp;
    char* input_msg;
    size_t input_msg_size = INPUT_MSG_SIZE;
    ssize_t input_bytes;

    input_msg = (char*)malloc(sizeof(char) * MSG_MAXSIZE);
    if (input_msg == NULL) {
        return NULL;
    }
    memset(input_msg, 0x00, MSG_MAXSIZE);

    // getline으로 메세지를 받아온다
    input_bytes = getline(&input_msg, &input_msg_size, stdin);
    if (input_bytes == -1) {
        goto getline_fail;
    }

    temp = init_chat_message();
    if (temp == NULL) {
        goto alloc_fail;
    }

    temp->msg_len = (uint32_t)input_bytes;
    temp->msg = input_msg;

    return temp;

alloc_fail:
getline_fail:
    free(input_msg);
    return NULL;
}

