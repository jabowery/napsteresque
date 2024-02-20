/*
 * $Id: socket.hh,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_SOCKET_HH
#define _NAPD_SOCKET_HH

#include <list>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
}

#include "data.hh"
#include "address.hh"
#include "command.hh"
#include "time.hh"

struct Socket {
  int fd;	// valid if opened()
  Address addr;	// valid if bound()
  Address peer;	// valid if connected()

  union Flags {
    struct {
      U8 opened		: 1;
      U8 bound		: 1;
      U8 connected	: 1;
      U8 listening	: 1;
    } bit;
    U8 all;
  } flags;

  typedef int Type;

  Socket()
    { flags.all = 0; }
  Socket(Type t)
    { flags.all = 0; open(t); }
  Socket(const Socket &s)
    { flags.all = 0; *this = s; }

  Socket &operator =(const Socket &s);

  ~Socket() {
    if (opened())
      close();
  }

  int _fcntl(int, long);

  template <class T> void _getopt(int l, int optname, T *optval) {
    socklen_t optlen = sizeof(*optval);
    if (::getsockopt(fd, l, optname, optval, &optlen) < 0)
      THROW;
  }

  template <class T> void _setopt(int l, int optname, const T& optval) {
    if (::setsockopt(fd, l, optname, &optval, sizeof(optval)) < 0)
      THROW;
  }

  Time linger();
  Time linger(const Time& t);

  bool nonblock();
  bool nonblock(bool b);

  int recvbuf();
  int recvbuf(int s);

  U8 recvlowat();
  U8 recvlowat(U8);

  int sendbuf();
  int sendbuf(int s);

  U8 sendlowat();
  U8 sendlowat(U8);

  bool broadcast();
  bool broadcast(bool b);

  U8 multicast_ttl();
  U8 multicast_ttl(U8 ttl);
  void multicast_add(const Address &);

  bool reuseaddr();
  bool reuseaddr(bool b);

  void open(Type = SOCK_STREAM);
  bool opened() { return flags.bit.opened; }

  Type type();

  void bind()
    { bind(Address::ANY); }
  void bind(const Address &a);
  void bind(U16 port)
    { bind(Address(0U, port)); }
  Address *bound() { return flags.bit.bound ? &addr : NULL; }

  void connect(const Address &a);
  Address *connected() { return flags.bit.connected ? &peer : NULL; }

  void listen(int backlog = 128);
  bool listening() { return flags.bit.listening; }

  void accept(Socket *s);

  int recv(U8 *buf, U32 len, int f = 0);
  int recv(U8 *buf, U32 len, Address *from, int f = 0);
  int recv(Data *d, int f = 0);
  int recv(Data *d, Address *from, int f = 0);
  int recv(Command *c, int f = 0);

  int send(U8 *buf, U32 len, int f = 0);
  int send(U8 *buf, U32 len, const Address &to, int f = 0);
  int send(const Data &d, int f = 0);
  int send(const Data &d, const Address &to, int f = 0);
  int send(Command *c, int f = 0);
  int send(const char *buf);

  void close();
  static void test();
};

typedef list<Socket*> SocketList;

#endif
