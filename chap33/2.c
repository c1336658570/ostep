//重要API：select()（或poll()）
//检查是否有任何应该关注的进入I/O。例如，假定网络应用程序（如Web服务器）希望检查是否有网络数据包已到达，以便为它们提供服务。

//阻塞与非阻塞接口　
//阻塞（或同步，synchronous）接口在返回给调用者之前完成所有工作。非阻塞（或异步，asynchronous）接口开始一些工作，
//但立即返回，从而让所有需要完成的工作都在后台完成。

//使用select()
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//初始化完成后，服务器进入无限循环。在循环内部，它使用FD_ZERO()宏首先清除文件描述符集合，
//然后使用FD_SET()将所有从minFD到maxFD的文件描述符包含到集合中。
//最后，服务器调用select()来查看哪些连接有可用的数据。然后，通过在循环中使用FD_ISSET()，
//事件服务器可以查看哪些描述符已准备好数据并处理传入的数据。

int main(void) {
  // open and set up a bunch of sockets (not shown)
  // main loop
  while (1) {
    // initialize the fd_set to all zero
    fd_set readFDs;
    FD_ZERO(&readFDs);

    // now set the bits for the descriptors
    // this server is interested in
    // (for simplicity, all of them from min to max)
    int fd;
    for (fd = minFD; fd < maxFD; fd++) 
      FD_SET(fd, &readFDs);

    // do the select
    int rc = select(maxFD + 1, &readFDs, NULL, NULL, NULL);

    // check which actually have data using FD_ISSET()
    int fd;
    for (fd = minFD; fd < maxFD; fd++)
      if (FD_ISSET(fd, &readFDs)) 
        processFD(fd);
  }
}