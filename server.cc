/*
 * $Id: server.cc,v 1.175.4.31.2.38 2001/08/24 02:19:17 dbrumleve Exp $
 */

#define _NAPD_SERVER_CC

#include <vector>
#include <slist>
#include <hash_map>
#include <iostream>
#include <iomanip>

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <pwd.h>
#include <malloc.h>
#include <stdarg.h>
}

#include "defines.hh"
#include "functions.hh"
#include "codes.hh"
#include "db.hh"
#include "filecert.hh"
#include "scheduler.hh"
#include "scheduled.hh"
#include "search.hh"
#include "client.hh"
#include "server.hh"
#include "remote.hh"
#include "thread.hh"

Server::Server() : Socket(), PollSet() {

}

Server::Server(U16 port) : Socket(), PollSet()
  { open(port); }

Server::Server(const Address &a) : Socket(), PollSet()
  { open(a); }

Server::~Server() {
  close();
}

static void *accept_loop(void *_server) {
  Server *server = (Server *)_server;

  server->incoming.unlock();

  while (1) {
    Client *c = new Client;
    try {
      server->accept(c);
      c->nonblock(1);
    } CATCH {
      delete c;
      LOG(WARNING, "%s", strerror(errno));
      continue;
    }

    server->incoming.lock();
    server->incoming->push_back(c);
    server->incoming.unlock();
  }

  Thread::test_cancel();
  return NULL;
}

void Server::open(const Address &a) {
  Socket::open(SOCK_STREAM);
  Socket::bind(a);

  Socket::listen();

  incoming.lock();
  accept_thread.spawn(accept_loop, (void *)this);
  accept_thread.detach();

  incoming.lock(); // wait for accept_thread to get going
  incoming.unlock();

  LOG(INFO, "opened socket on port %hu", addr.port);
}

void Server::open(U16 port)
  { open(Address(0U, port)); }

void Server::close() {
  accept_thread.cancel();
  Socket::close();
}

void Server::poll() {
  foreach (i, clients)
    watch(**i);

  PollSet::poll();
}

void Server::check_incoming(void) {
  incoming.lock();
  foreach (i, *incoming)
    add_connection(*i);
  incoming->clear();
  incoming.unlock();
}

void Server::add_connection(Client *lc) {
  // first verify that they are not already in the queue
  // if they are, honor the most recent connection

  foreach (i, clients)
    if ((*i)->ip() == lc->ip()) {
      LOG(NOTICE, "%s was already in queue, removing", (*i)->peer.ip_str());
      (*i)->flags.bit.dead = 1;
    }

  LOG(INFO, "%s", lc->ip_str());
  clients.push_back(lc);
}

void Server::remove_connection(ClientList::iterator i) {
  Client *c = *i;
  clients.erase(i);
  LOG(INFO, "%s", c->ip_str());
  delete c;
}

void Server::check_clients() {
  poll();

  ClientList::iterator i = clients.begin();

  while (i != clients.end()) {
    Client *lc = *i;

    try {
      if (lc->flags.bit.dead)
        THROW;

      // data waiting to come in?
      if (can_read(*lc))
        if (Command *com = lc->read()) {
          LOG(DEBUG, "%d (size %d)", com->code, com->size);
          delete com;
        }

      // data waiting to go out?
      if (can_write(*lc))
        lc->flush();

      i++;
    } CATCH {
      remove_connection(i++);
    }
  }
}

void Server::test() {
  LOG(INFO);

  Server s(6666);		assert(s.bound() && s.listening());
  Client c(s.addr);		assert(c.connected());
  				assert(s.clients.empty());
  while (s.clients.empty()) {
    Thread::yield();
    s.check_incoming();
  }

  Client &d = *s.clients.front();

  SAY(&c, (Command::Code)0, 1, 2, 3);
  s.poll();			assert(s.can_read(d));
  Command *com = d.read();	assert(!strcmp((char*)com->data, "1 2 3"));
  delete com;

  c.close();
  s.check_clients();		assert(s.clients.empty());
				assert(s.bound() && s.listening());
				assert(!c.connected());
}
