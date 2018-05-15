#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "server.h"


/*
 * 자식 프로세스는 데몬으로 서비스 하여 server함수를 동작.
 */
int main(int argc, char** argv) 
{
    enum errnum err;
    pid_t pid = 0;

    pid = fork();
	if (pid < 0) { // background
		perror("fork error");
	}
	else if (pid > 0) {
		return 0;
	}
	else {
        err = run_server();
        printf("%s\n", find_err_msg(err));
        printf("done\n");
    }
    
	return 0;
}
