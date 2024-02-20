/* 
 * $Id: server.hh,v 1.64.4.4.2.21 2001/08/24 02:19:18 dbrumleve Exp $
 */

#ifndef _NAPD_SERVER_HH
#define _NAPD_SERVER_HH

#include <list>
#include <vector>
#include <iostream>

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
}

#include "defines.hh"
#include "socket.hh"
#include "client.hh"
#include "channel.hh"
#include "thread.hh"
#include "mutex.hh"

#include "pollset.hh"

class RemoteServer;
class RemoteClient;

struct Server : Socket, PollSet {
  Thread accept_thread;
  Lockable<ClientList> incoming;
  ClientList clients;

 public:
  Server();
  Server(const Address &);
  Server(U16 port);
  ~Server();

  void open(const Address &);
  void open(U16);
  void close();

  void poll();

  void check_clients(void);
  void check_incoming(void);

  void add_connection(Client *);
  void remove_connection(ClientList::iterator);

  static void test();
};

#endif
