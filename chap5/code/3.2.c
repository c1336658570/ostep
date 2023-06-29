/*
使用信号处理函数实现父进程不wait，但是子进程永远先执行。
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <bits/sigaction.h>
#include <asm-generic/signal-defs.h>

volatile sig_atomic_t child_done = 0;

void handle_sigchld(int sig) {
    child_done = 1;
}

int main() {
  pid_t pid = fork();

  if (pid < 0) {
    fprintf(stderr, "Fork failed");
    exit(1);
  } else if (pid == 0) {
    // 子进程
    printf("hello\n");
    exit(0);
  } else {    //有一个问题，就是当父进程还未注册信号处理函数子进程结束执行了
    // 父进程
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
      fprintf(stderr, "Failed to set signal handler");
      exit(1);
    }
    while (!child_done) {
      // 等待子进程结束
    }
    printf("goodbye\n");
  }

    return 0;
}