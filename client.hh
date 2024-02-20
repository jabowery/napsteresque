/*
 * $Id: client.hh,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_CLIENT_HH
#define _NAPD_CLIENT_HH

#include "defines.hh"
#include "command.hh"
#include "socket.hh"

struct Client : Socket {
  Command *rbuffer;
  Time rbuffer_age;

  list<Command *> wqueue;
  Time wqueue_age;

  // also Socket::flags
  union Flags {
    struct {
      U8 flooding	: 1;
      U8 secure		: 1;
      U8 dead		: 1;
    } bit;
    U8 all;
  } flags;

  void _clear();
  Client();
  Client(Socket::Type type);
  Client(const Address &, Socket::Type type = SOCK_STREAM);
  Client(const Address &, const Address &, Socket::Type type = SOCK_STREAM);
  ~Client();

  Command *read();
  void write(Command *);
  void flush();

  void open(Socket::Type = SOCK_STREAM);
  void open(const Address &, Socket::Type = SOCK_STREAM);
  void open(const Address &, const Address &, Socket::Type type = SOCK_STREAM);
  void close();

  U32 ip() const
    { return Socket::peer.ip; }
  char *ip_str() const
    { return Socket::peer.ip_str(); }

  friend Client& operator << (Client &, Command *);
  friend Client& operator >> (Client &, Command *);

  static void test();
};

typedef list<Client *> ClientList;

#endif
