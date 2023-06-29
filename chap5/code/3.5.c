/*
在Linux中，可以使用POSIX线程库提供的互斥锁（mutex）和条件变量（condition variable）来实现进程间的同步。
这些锁和条件变量可以在多个进程之间共享，因此可以用于进程间的同步。
为了在多个进程之间共享锁和条件变量，需要使用共享内存来存储这些锁和条件变量。可以使用mmap()系统调用创建共享内存区域，
并将锁和条件变量存储在共享内存中。不同进程可以通过共享内存来访问和操作这些锁和条件变量。

使用mmap()系统调用创建了一个共享内存区域，其中包含一个互斥锁和一个标志位。
我们使用pthread_mutexattr_setpshared()函数设置互斥锁为进程间共享，并使用pthread_mutex_init()函数初始化互斥锁。
在子进程中，我们使用互斥锁对标志位进行保护，并在子进程结束时将标志位设置为1。
在父进程中，我们使用while循环等待标志位变成1，即等待子进程结束。在父进程中，我们使用互斥锁对标志位进行保护，
并在父进程结束时销毁互斥锁和共享内存。
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    int done;
} shared_data_t;

int main() {
    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        fprintf(stderr, "Failed to create shared memory");
        exit(1);
    }

    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shared_data->mutex), &mutexattr);

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        pthread_mutex_lock(&(shared_data->mutex));
        printf("hello\n");
        shared_data->done = 1;
        pthread_mutex_unlock(&(shared_data->mutex));
        exit(0);
    } else {
        // 父进程
        while (1) {
            pthread_mutex_lock(&(shared_data->mutex));
            if (shared_data->done) {
                break;
            }
            pthread_mutex_unlock(&(shared_data->mutex));
        }
        printf("goodbye\n");
        pthread_mutex_unlock(&(shared_data->mutex));
        pthread_mutex_destroy(&(shared_data->mutex));
        munmap(shared_data, sizeof(shared_data_t));
    }

    return 0;
}