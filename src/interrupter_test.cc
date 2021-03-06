/** @file
 * @brief interrupter test
 * @author zhangyafeikimi@gmail.com
 * @date
 * @version
 *
 */
#include "ev.h"
#include "log.h"
#include "interrupter.h"
#include "header.h"
#include <pthread.h>

using namespace libev;

static Interrupter inter;
static const int times = 10;

static void * ThreadFunc(void *)
{
  int i;
  for (i=0; i<times; i++)
  {
    sleep(1);
    EV_VERIFY(inter.Interrupt() == kEvOK);
  }

  return 0;
}

int main()
{
  int i;
  int result;
  int epfd;
  epoll_event event;
  pthread_t tid;

  result = inter.Init();
  EV_VERIFY(result == kEvOK);

  epfd = epoll_create(10);
  EV_VERIFY(epfd != -1);

  memset(&event.data, 0, sizeof(event.data));
  event.data.fd = inter.fd();
  event.events = EPOLLIN|EPOLLET;

  result = epoll_ctl(epfd, EPOLL_CTL_ADD, inter.fd(), &event);
  EV_VERIFY(result == 0);

  result = pthread_create(&tid, 0, ThreadFunc, 0);
  EV_VERIFY(result == 0);

  for (i=0; i<times; i++)
  {
    EV_LOG(kInfo, "waiting");
    result = epoll_wait(epfd, &event, 1, -1);
    if (result == -1 && errno == EINTR)
      continue;

    EV_VERIFY(result == 1);
    EV_VERIFY(event.data.fd == inter.fd());
    EV_LOG(kInfo, "interrupted");
    inter.Reset();
  }

  EV_LOG(kInfo, "quit");

  result = pthread_join(tid, 0);
  EV_VERIFY(result == 0);

  safe_close(epfd);
  inter.UnInit();
  return 0;
}
