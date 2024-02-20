/*
 * $Id: pollset.hh,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_POLLSET_HH
#define _NAPD_POLLSET_HH

extern "C" {
#include <sys/poll.h>
}

#include "defines.hh"
#include "socket.hh"
#include "time.hh"

class PollSet {
  // private (!)
  U32 *index;
  struct pollfd *table;
  U32 size;

public:
  PollSet() {
    int cap;
    if ((cap = getdtablesize()) < 0)
      throw errno;
    size = 0;
    index = new U32[cap];
    table = new struct pollfd[cap];
  }

  ~PollSet() {
    delete[] index;
    delete[] table;
  }

  void watch(const Socket &s, int events = POLLIN | POLLOUT) {
    table[size].fd = s.fd;
    table[size].events = events;
    table[size].revents = 0;
    index[s.fd] = size++;
  }

  bool ready(const Socket &s, int events) const {
    int ret = !!(table[index[s.fd]].revents & events);
    return ret;
  }

  bool can_read(const Socket &s) const
    { return ready(s, POLLIN); }
  bool can_write(const Socket &s) const
    { return ready(s, POLLOUT); }

  void poll(Time timeout = 0.001) {
    int ret = ::poll(table, size, timeout.msecs());
    size = 0;
    if (ret < 0)
      throw errno;
  }

  static void test();
};

#endif
