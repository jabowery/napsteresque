/* 
 * $Id: socket.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_SOCKET_CC

#include "socket.hh"
#include "defines.hh"

Socket &Socket::operator =(const Socket &s) {
  if (opened())
    close();

  // we use dup here since we close on destruction (?)
  if ((fd = ::dup(s.fd)) < 0)
    THROW;

  addr = s.addr;
  peer = s.peer;
  flags.all = s.flags.all;

  return *this;
}

void Socket::open(Socket::Type _type = SOCK_STREAM) {
  if (opened())
    close();

  if ((fd = socket(AF_INET, _type, 0)) < 0)
    THROW;
  flags.bit.opened = 1;
}

int Socket::_fcntl(int cmd, long arg) {
  int ret;
  if ((ret = ::fcntl(fd, cmd, arg)) < 0)
    THROW;
  return ret;
}

Time Socket::linger() {
  struct linger lin;
  _getopt(SOL_SOCKET, SO_LINGER, &lin);
  return Time(lin.l_onoff ? lin.l_linger : 0);
}

Time Socket::linger(const Time& t) {
  struct linger lin;
  memset(&lin, 0, sizeof(lin));
  lin.l_onoff = !!t;
  lin.l_linger = t.secs();
  _setopt(SOL_SOCKET, SO_LINGER, lin);
  return t;
}

bool Socket::nonblock() {
  return _fcntl(F_GETFL, 0) & O_NONBLOCK;
}

bool Socket::nonblock(bool b) {
  int f = _fcntl(F_GETFL, 0);
  if (b) f |=  O_NONBLOCK;
  else   f &= ~O_NONBLOCK;
  _fcntl(F_SETFL, f);
  return !!b;
}

Socket::Type Socket::type() {
  int val;
  _getopt(SOL_SOCKET, SO_TYPE, &val);
  return val;
}

int Socket::recvbuf() {
  int val;
  _getopt(SOL_SOCKET, SO_RCVBUF, &val);
  return val;
}

int Socket::recvbuf(int s) {
  _setopt(SOL_SOCKET, SO_RCVBUF, s);
  return s;
}

U8 Socket::recvlowat(U8 l) {
  _setopt(SOL_SOCKET, SO_RCVLOWAT, l);
  return l;
}

U8 Socket::recvlowat() {
  U8 l;
  _getopt(SOL_SOCKET, SO_RCVLOWAT, &l);
  return l;
}

int Socket::sendbuf() {
  int val;
  _getopt(SOL_SOCKET, SO_SNDBUF, &val);
  return val;
}

int Socket::sendbuf(int s) {
  _setopt(SOL_SOCKET, SO_SNDBUF, s);
  return s;
}

U8 Socket::sendlowat(U8 l) {
  _setopt(SOL_SOCKET, SO_SNDLOWAT, l);
  return l;
}

U8 Socket::sendlowat() {
  U8 l;
  _getopt(SOL_SOCKET, SO_SNDLOWAT, &l);
  return l;
}

bool Socket::reuseaddr() {
  int val;
  _getopt(SOL_SOCKET, SO_REUSEADDR, &val);
  return !!val;
}

bool Socket::reuseaddr(bool b) {
  _setopt(SOL_SOCKET, SO_REUSEADDR, (int)b);
  return !!b;
}

bool Socket::broadcast() {
  int val;
  _getopt(SOL_SOCKET, SO_BROADCAST, &val);
  return !!val;
}

bool Socket::broadcast(bool b) {
  _setopt(SOL_SOCKET, SO_BROADCAST, (int)b);
  return !!b;
}

U8 Socket::multicast_ttl(U8 ttl) {
  _setopt(IPPROTO_IP, IP_MULTICAST_TTL, ttl);
  return ttl;
}

U8 Socket::multicast_ttl() {
  U8 ttl;
  _getopt(IPPROTO_IP, IP_MULTICAST_TTL, &ttl);
  return ttl;
}

void Socket::multicast_add(const Address &group) {
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = htonl(group.ip);
  mreq.imr_interface.s_addr = INADDR_ANY;
  _setopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq);
}

void Socket::bind(const Address &a) {
  sockaddr_in sin = a;
  socklen_t sin_len = sizeof(sin);

  reuseaddr(1);

  int ret;
  if ((ret = ::bind(fd, (sockaddr *)&sin, sizeof(sockaddr))) < 0)
    THROW;
  if (::getsockname(fd, (sockaddr *)&sin, &sin_len) < 0)
    THROW;

  addr = sin;
  flags.bit.bound = 1;

  // automagic
  if (addr.multicast())
    multicast_add(addr);
}

void Socket::connect(const Address &a) {
  sockaddr_in sin = a;
  socklen_t sin_len = sizeof(sockaddr_in);

  int ret;
  if ((ret = ::connect(fd, (sockaddr *)&sin, sizeof(sockaddr))) < 0)
    THROW;

  flags.bit.connected = 1;
  peer = sin;

  if (!flags.bit.bound && !::getsockname(fd, (sockaddr *)&sin, &sin_len)) {
    flags.bit.bound = 1;
    addr = sin;
  }
}

void Socket::listen(int backlog = 128) {
  int ret;
  if ((ret = ::listen(fd, backlog)) < 0)
    THROW;
  flags.bit.listening = 1;
}

void Socket::accept(Socket *s) {
  sockaddr_in speer;
  socklen_t speer_len = sizeof(speer);

  int sfd;
  if ((sfd = ::accept(fd, (sockaddr *)&speer, &speer_len)) < 0)
    THROW;

  if (s->opened())
    s->close();

  s->flags.all			= 0;
  s->flags.bit.opened		= 1;
  s->flags.bit.bound		= 1;
  s->flags.bit.connected	= 1;
  s->fd				= sfd;
  s->addr			= addr;
  s->peer			= speer;
}


int Socket::recv(U8 *buf, U32 len, int f = 0) {
  return ::recv(fd, buf, len, f);
}

int Socket::recv(U8 *buf, U32 len, Address *from, int f = 0) {
  sockaddr_in sin; socklen_t sin_len = sizeof(sin);
  int ret = ::recvfrom(fd, buf, len, f, (sockaddr *)&sin, &sin_len);
  *from = sin;
  return ret;
}

int Socket::recv(Data *d, int f = 0)
  { return recv(d->data, d->size, f); }
int Socket::recv(Data *d, Address *from, int f = 0)
  { return recv(d->data, d->size, from, f); }

int Socket::recv(Command *c, int f = 0) {
  Data d(c->data + c->offset, c->size - c->offset);
  int ret = connected() ? recv(&d, f) : recv(&d, &c->peer, f);
  if (ret < 1)
    return ret;
  c->offset += ret;
  return ret;
}

int Socket::send(U8 *buf, U32 len, int f = 0) {
  return ::send(fd, buf, len, f);
}

int Socket::send(U8 *buf, U32 len, const Address &to, int f = 0) {
  sockaddr_in sin = to;
  int ret = ::sendto(fd, buf, len, f, (sockaddr *)&sin, sizeof(sin));
  return ret;
}

int Socket::send(const Data &d, int f = 0)
  { return send(d.data, d.size, f); }

int Socket::send(const Data &d, const Address &to, int f = 0)
  { return send(d.data, d.size, to, f); }
int Socket::send(const char *buf)
  { return send((U8*)buf, strlen(buf)); }

int Socket::send(Command *c, int f = 0) {
  Data d(c->data, c->size);
  d += c->offset;

  int ret = (bool)c->peer ? send(d, c->peer, f) : send(d, f);
  if (ret < 1)
    return ret;
  c->offset += ret;
  return ret;
}

void Socket::close() {
  flags.all = 0;
  ::close(fd);
  fd = -1;
}

void Socket::test() {
  LOG(INFO);

  Socket s(SOCK_STREAM);		assert(s.opened());
  s.bind(1234);				assert(s.addr.port == 1234);

  Socket t = s;				assert(t.fd != s.fd);
  Socket u = t;				assert(u.fd > t.fd); // see dup(2)
					assert(s.addr == t.addr);
					assert(t.opened());
  try {
    Socket m(SOCK_DGRAM);
    m.bind("224.6.6.6:9999");

    Socket n(SOCK_DGRAM);
    n.bind();

    int ret;

    Data d("hello world\n", 13);
    ret = n.send(d, m.addr);	assert(ret == (int)d.size);

    Data f(new U8[d.size], d.size);
    Address from;
    ret = m.recv(&f, &from);	assert(ret == (int)f.size);
				assert(from.port == n.addr.port);
				assert(d == f);
  } CATCH {
    LOG(ERR, "%s", ERRSTR);
    exit(1);
  }
}
