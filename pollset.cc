/* 
 * $Id: pollset.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_POLLSET_CC

#include <vector>

#include "defines.hh"
#include "pollset.hh"
#include "socket.hh"

void PollSet::test() {
  LOG(INFO);
  PollSet p;
  try {
    Socket sl[10];

    for (int i = 0; i < 10; i++) {
      Socket *s = sl + i;
      s->open(SOCK_STREAM);
      s->bind(Address(0U, 12345 + i));
      s->nonblock(1);
      s->listen(10);
    }

    Socket a(SOCK_STREAM);
    a.connect(Address(0U, 12345));

    Socket b(SOCK_STREAM);
    b.connect(Address(0U, 12350));

    for (int i = 0; i < 10; i++)
      p.watch(sl[i]);
    p.watch(a);
    p.watch(b);

    p.poll();
  					assert(p.can_read(sl[0]));
					assert(p.can_read(sl[5]));
					assert(!p.can_read(sl[1]));
					assert(!p.can_read(sl[4]));
					assert(!p.can_read(a));
					assert(p.can_write(a));
					assert(!p.can_read(b));
					assert(p.can_write(b));
    Socket c, d;
    sl[0].accept(&c);
    sl[5].accept(&d);

    c.send("hello");
    d.send("world");

    for (int i = 0; i < 10; i++)
      p.watch(sl[i], POLLIN | POLLOUT);
    p.watch(a);
    p.watch(b);

    p.poll();
					assert(p.can_read(a));
					assert(p.can_write(a));
					assert(p.can_read(b));
					assert(p.can_write(b));
					assert(!p.can_read(sl[0]));
					assert(!p.can_read(sl[5]));
  } CATCH {
    LOG(ERR, "%s", strerror(errno));
    assert(0);
  }
}
