/*
 * $Id: address.cc,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_ADDRESS_CC

#include "defines.hh"
#include "address.hh"

const Address Address::ANY(0U, 0U);

// this is a wrapper around struct sockaddr_in for use with Socket.

Address::Address(const char *host, U16 _port = 0) {
  char buf[256];

  if (_port)
    port = _port;
  else if (char *p = strchr(host, ':')) {
    strncpy(buf, host, sizeof(buf) - 10);
    buf[sizeof(buf) - 1] = '\0';
    *(buf + (p - host)) = 0;
    port = atoi(p + 1);
    host = buf;
  } else
    port = 0;

  if (!*host)
    ip = 0;
  else if ((ip = inet_addr(host)) == INADDR_NONE) {
    struct hostent *he = gethostbyname(host);
    if (!he)
      THROW;
    ip = *(U32 *)he->h_addr;
  }
  ip = ntohl(ip);
}

Address::Address(const sockaddr_in &sin) {
  ip	= ntohl(sin.sin_addr.s_addr);
  port	= ntohs(sin.sin_port);
}

Address &Address::operator =(const sockaddr_in &sin) {
  ip	= ntohl(sin.sin_addr.s_addr);
  port	= ntohs(sin.sin_port);
  return *this;
}

Address::Address(const Address &a) {
  ip	= a.ip;
  port	= a.port;
}

Address &Address::operator =(const Address &a) {
  ip	= a.ip;
  port	= a.port;
  return *this;
}

bool Address::operator ==(const Address &a) const {
  return (a.ip == ip && a.port == port);
}

bool Address::operator !=(const Address &a) const {
  return (a.ip != ip || a.port != port);
}

Address::operator sockaddr_in() const {
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family	= AF_INET;
  sin.sin_addr.s_addr	= htonl(ip);
  sin.sin_port		= htons(port);
  return sin;
}

// rotating 8k buffer.  the return value will be overwritten 256
// invocations of this function later.  still, copy this data
// elsewhere right away unless you know what you're doing1

const static U32 _ip_size	= 32;
const static U32 _buffer_size	= _ip_size * 256;
static U32 _buffer_pos		= 0;
static char _buffer[_buffer_size];

char *Address::ip_str() const {
  if (_buffer_pos >= _buffer_size)
    _buffer_pos = 0;

  char *ret = _buffer + _buffer_pos;
  in_addr in; in.s_addr = ntohl(ip);

  strncpy(ret, inet_ntoa(in), _ip_size - 1);
  _buffer_pos += _ip_size;
  *(_buffer + _buffer_pos - 1) = '\0';

  return ret;
}

Address::operator char *() const {
  char *ret = ip_str();
  sprintf(ret + strlen(ret), ":%hu", port);
  return ret;
}

unsigned Address::dotcount(const char *str) {
  unsigned count = 0;
  const char *s = str;
  char *token = NULL;

  while (*s && (token = strchr(s, '.'))) {
    s = token + 1;
    count++;
  }

  return count;
}

void Address::test() {
  LOG(INFO);

  Address l("127.0.0.1:99");	assert(l == Address("127.0.0.1", 99));
  char _ip[4] = {127,0,0,1};
  U32 ip = ntohl(*(U32 *)_ip);	assert(l.ip == ip);
				assert(l.port == 99);
  				assert(!strcmp(l, "127.0.0.1:99"));
				assert(!strcmp(l.ip_str(), "127.0.0.1"));
				assert(dotcount(l.ip_str()) == 3);
  int i = 0;
  char *str = l.ip_str();
  while (str < l.ip_str())
    i++;
				assert(i < 256);
  sockaddr_in sin = l;		assert(sin.sin_port == htons(l.port));
				assert(sin.sin_addr.s_addr == htonl(l.ip));

  				assert(Address("224.0.0.0").multicast());
  				assert(Address("239.255.255.255").multicast());
				assert(!l.multicast());
}
