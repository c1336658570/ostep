//
// Created by pjs on 2020/10/7.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd[2];
  if (pipe(fd) < 0) {
    perror("pipe");
    exit(1);
  }


  int pid = fork();
  if (pid > 0) {
    int ppid = fork();
    if (ppid == 0) {
      char *msg = "Hello!\n";
      //关闭多余管道端
      close(fd[0]);
      //关闭标准输出
      close(STDOUT_FILENO);
      dup(fd[1]);
      write(STDOUT_FILENO, msg, sizeof(msg));
      close(fd[1]);
    } else if (ppid > 0) {
      close(fd[0]);
      close(fd[1]);
      wait(NULL);
      wait(NULL);
      printf("parent");
    }
  } else if (pid == 0) {
    close(fd[1]);
    close(STDIN_FILENO);
    char buf[10];
    memset(buf, 0, sizeof(buf));
    dup(fd[0]);
    read(STDIN_FILENO, buf, sizeof(buf));
    sprintf(buf, "%s", buf);
    int fd1 = open("./8test.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR |S_IWUSR);
    write(fd1, buf, strlen(buf));
  }
  return 0;
}