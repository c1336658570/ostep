/*
使用管道实现父进程不wait，但是子进程永远先执行。
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0) {
        fprintf(stderr, "pipe error\n");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (pid == 0) {  // 子进程
        close(fd[0]);  // 关闭读端
        printf("hello\n");
        write(fd[1], "1", 1);  // 向管道写入数据
        close(fd[1]);  // 关闭写端
        exit(0);
    } else {  // 父进程
        close(fd[1]);  // 关闭写端
        char buf[1];
        read(fd[0], buf, 1);  // 从管道读取数据
        printf("goodbye\n");
        close(fd[0]);  // 关闭读端
        exit(0);
    }
}