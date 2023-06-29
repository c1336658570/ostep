/*
使用共享内存和信号量来实现在父进程不调用wait()的情况下确保子进程先打印。

使用mmap()系统调用创建了一段共享内存，并使用sem_open()系统调用创建了一个信号量。
父进程在一个while循环中等待done变成1，即等待子进程结束。在子进程中，我们将done标记为1，
并使用sem_post()函数释放信号量。在父进程中，我们使用sem_wait()函数等待信号量，当子进程释放信号量时，
父进程会继续执行，打印"goodbye"。
使用共享内存和信号量等待子进程结束的好处是可以避免父进程调用wait()函数而被阻塞，
并且可以避免管道的缓冲区大小和竞态条件等问题，但需要注意共享内存和信号量的使用和清理。
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    int *done = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (done == MAP_FAILED) {
        fprintf(stderr, "Failed to create shared memory");
        exit(1);
    }

    sem_t *sem = sem_open("/mysem", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Failed to create semaphore");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("hello\n");
        *done = 1;
        sem_post(sem);
        exit(0);
    } else {
        // 父进程
        while (!(*done)) {
            sem_wait(sem);
        }
        printf("goodbye\n");
        sem_close(sem);
        sem_unlink("/mysem");
        munmap(done, sizeof(int));
    }

    return 0;
}