/*
 * $Id: address.hh,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_ADDRESS_HH
#define _NAPD_ADDRESS_HH

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
}

#include "defines.hh"

struct Address {
  U32 ip;
  U16 port;

  Address(U32 _ip = 0, U16 _port = 0) : ip(_ip), port(_port) { }

  Address(const Address &a);
  Address(const char *host, U16 _port = 0);
  Address &operator =(const Address &a);

  Address(const sockaddr_in &sin);
  Address &operator =(const sockaddr_in &sin);

  operator sockaddr_in() const;
  operator char *() const;
  operator bool() const { return ip || port; }

  bool operator ==(const Address &) const;
  bool operator !=(const Address &) const;

  char *ip_str() const;

  bool multicast() const { return ip>>28 == 14; }

  static unsigned dotcount(const char *);
  static void test();

  const static Address ANY;
};

#endif
