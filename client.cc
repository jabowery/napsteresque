/*
 * $Id: client.cc,v 1.18.4.5.2.9 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_CLIENT_CC

extern "C" {
#include <errno.h>
}

#include "client.hh"
#include "defines.hh"

void Client::_clear() {
  rbuffer = NULL;
  flags.all = 0;
  rbuffer_age = 0;
  wqueue_age = 0;
}

Client::Client() : Socket() {
  _clear();
}

Client::Client(Socket::Type type) : Socket() {
  open(type);
}

Client::Client(const Address &p, Socket::Type type = SOCK_STREAM) : Socket() {
  open(p, type);
}

Client::Client(const Address &x, const Address &y, Socket::Type type = SOCK_STREAM) {
  open(x, y, type);
}

Client::~Client() {
  close();
}

void Client::open(const Address &x, const Address &y, Socket::Type type = SOCK_STREAM) {
  _clear();
  Socket::open(type);
  bind(x);
  connect(y);
}

void Client::open(const Address &y, Socket::Type type = SOCK_STREAM) {
  _clear();
  Socket::open(type);
  connect(y);
}

void Client::open(Socket::Type type = SOCK_STREAM) {
  _clear();
  Socket::open(type);
}

Command *Client::read() {
  if (!rbuffer) {
    CommandHead h;
    Address from;

    int ret;
    if (connected())
      ret = recv((U8 *)&h, sizeof(h), MSG_PEEK);
    else
      ret = recv((U8 *)&h, sizeof(h), &from, MSG_PEEK);

    if (!ret || ret < 0 && errno != EAGAIN && errno != EINTR)
      THROW;
    if (ret != sizeof(h))
      return NULL;

    h.ntoh();
    
    rbuffer = new Command;
#warning "kludge"
    rbuffer->_grow(h.size);
    rbuffer->code = h.code;
    rbuffer->size = h.size;
    rbuffer->offset = -sizeof(h);

    rbuffer_age = Time::now();
  }

  while (!rbuffer->full()) {
    int ret = recv(rbuffer);
    if (!ret || ret < 0 && errno != EAGAIN && errno != EINTR)
      THROW;
    if (ret < 0)
      return NULL;
  }

  Command *com = rbuffer;
  com->offset = 0;

  rbuffer = NULL;
  rbuffer_age = 0;

  return com;
}

void Client::flush() {
  foreach_erase (i, wqueue) {
    Command *com = *i;

    while (!com->full()) {
      int ret = send(com);

      if (!ret || ret < 0 && errno != EAGAIN && errno != EINTR)
        THROW;
      if (ret < 0)
        return;
    }

    delete com;
  }

  wqueue_age = 0;
}

void Client::write(Command *c) {
  if (!c->data)
    c->data = (U8 *)(new CommandHead + 1);

  CommandHead *h = (CommandHead *)(c->data - sizeof(CommandHead));
  h->size = htons(c->size);
  h->code = htons(c->code);

  bool was_empty = wqueue.empty();
  wqueue.push_back(c);
  c->offset = -sizeof(CommandHead);

  if (!wqueue_age)
    wqueue_age = Time::now();

  if (was_empty)
    flush();
}

void Client::close() {
  if (rbuffer)
    delete rbuffer;
  foreach (i, wqueue)
    delete *i;
  wqueue.clear();

  Socket::close();
}

Client& operator << (Client &c, Command *com) {
  c.write(com);
  return c;
}

Client& operator >> (Client &c, Command *&com) {
  if (!(com = c.read()))
    THROW;
  return c;
}

void Client::test() {
  LOG(INFO);

  try {
    Client a(SOCK_DGRAM);
    a.bind("10.0.0.1:5555");
    a.connect("10.0.0.1:6666");

    Client b(SOCK_DGRAM);
    b.bind("10.0.0.1:6666");
    b.connect("10.0.0.1:5555");

    for (int i = 0; i < 10; i++) {
      Command *com = new Command(19);
      PACK_COMMAND(com, 1, i, 3);
      a.write(com);				assert(a.wqueue.empty());

      {
        Command *com = b.read();		assert(com);
						assert(com->code == 19);
        int l1, li, l3;				assert(com->size == 5);
        UNPACK_COMMAND(com, &l1, &li, &l3);	assert(l1==1 && li==i && l3==3);
      }
    }
  } CATCH {
    LOG(ERR, "%s", ERRSTR);
    assert(0);
  }
}

