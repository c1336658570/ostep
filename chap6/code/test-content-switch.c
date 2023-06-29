/*
用于测试上下文切换时间的C程序。程序使用管道进行进程间通信，并使用先来先服务（SCHED_FIFO）调度策略。
这段代码还使用了CPU亲和性设置，将进程绑定到CPU核心0上运行。
*/

#define _GNU_SOURCE // 使用GNU扩展功能，例如sched_setaffinity

#include "sched.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/time.h"
#include "unistd.h"

int main() {
    int fd1[2], fd2[2], fdt[2], pid, n;

    struct timeval tv_bg, tv_end;
    const struct sched_param param = {sched_get_priority_min(SCHED_FIFO)};
    cpu_set_t set;
    // 将CPU 0添加到集合
    CPU_SET(0, &set);

    printf("输入测试次数(次数太少会导致误差极大): ");
    scanf("%d", &n);

    // 创建三个管道
    if (pipe(fd1) < 0 || pipe(fd2) < 0 || pipe(fdt) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    };

    // 创建子进程
    if ((pid = fork()) < 0) {
        printf("error");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 需要root权限
        // 设置子进程为先来先服务调度策略（SCHED_FIFO）
        if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
            printf("程序运行需要root权限(sched_setscheduler系统调用)\n");
            perror("sched_setscheduler");
            exit(EXIT_FAILURE);
        }

        // 将子进程绑定到CPU核心0上
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }

        // 获取开始时间
        gettimeofday(&tv_bg, NULL);
        for (int i = 0; i < n; i++) {
            read(fd1[0], NULL, 1);
            write(fd2[1], "h", 1);
        }

        // 将开始时间写入管道
        write(fdt[1], &tv_bg, sizeof(tv_bg));
        exit(EXIT_SUCCESS);

    } else {
        // 设置父进程为先来先服务调度策略（SCHED_FIFO）
        if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
            perror("sched_setscheduler");
            exit(EXIT_FAILURE);
        }

        // 将父进程绑定到CPU核心0上
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }

        // 进行n次循环进行进程间通信
        for (int i = 0; i < n; i++) {
            write(fd1[1], "h", 1);
            read(fd2[0], NULL, 1);
        }
        // 获取结束时间
        gettimeofday(&tv_end, NULL);

        // 从管道中读取开始时间
        read(fdt[0], &tv_bg, sizeof(tv_bg));

        // 计算并打印上下文切换时间
        printf("上下文切换时间为: %f 微秒",
               (float) (tv_end.tv_sec * 1000000 + tv_end.tv_usec - tv_bg.tv_sec * 1000000 - tv_bg.tv_usec) / 2 / n);
    }
    return 0;
}