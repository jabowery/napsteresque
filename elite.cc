/* 
 * $Id: elite.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_ELITE_CC

#if 0
extern "C" {
#include <malloc.h>
}

#include "server.hh"
#include "functions.hh"
#include "file.hh"
#include "build.hh"
#include "scheduled.hh"

extern list<Client*> SearchQueue;

void Server::UserMute(Client *caller, Command *com) {
  if (caller->level < User::ELITE)
    return;
  caller->User::flags.bit.muted ^= 1;
}

void Server::UserCloak(Client *caller, Command *com) {
  if (caller->level < User::ELITE)
    return;
  caller->User::flags.bit.cloaked ^= 1;
}

void Server::Shutdown(Client *caller, Command *com) {
  if (caller->level < User::ELITE)
    return;
  exit(0);
}

#endif
