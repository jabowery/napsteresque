/*
 * $Id: notify.cc,v 1.20.20.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_NOTIFY_CC

#include <map>

#include "defines.hh"
#include "codes.hh"
#include "functions.hh"
#include "client.hh"
#include "remote.hh"
#include "server.hh"
#include "notify.hh"

#if 0

extern Server server;

NotifyManager::NotifyManager(void) { }

NotifyManager::~NotifyManager(void) {
  foreach (i, *this)
    delete (*i).first;
}

bool NotifyManager::remove(const char *notifier, Client *requester) {
  NotifyIterPair p = equal_range(notifier);
  
  if (p.first == p.second) {  // no entries found
    LOG(DEBUG, "%s:%s didn't exist", notifier, requester->name);
    return false;
  }

  // erase all key::value pairs, and note how many for debugging purposes
  char *oldkey = (char *)(p.first)->first;
  int found = 0;

  NotifyMap::iterator i = p.first;
  while (i != p.second)
    if ((*i).second == requester) {
      LOG(DEBUG, "removing %s:%s", oldkey, requester->name);
      erase(i++); 
      found++;
    } else i++;

  if (!found)
    LOG(DEBUG, "%s:%s not found", oldkey, requester->name);
  else { 
    if (found > 1)
      LOG(DEBUG, "more than one %s:%s found", oldkey, requester->name);

    // remove entry from client keyref list
    foreach (j, requester->notifies)
      if (*j == oldkey) {
	requester->notifies.erase(j);
	break;
      }
  }
  
  // finally, if that was the last key:value pair, free the memory.
  if (find(oldkey) == end()) {
    LOG(DEBUG, "deleting key for %s:%s", oldkey, requester->name);
    delete[] oldkey;
  }

  return (bool)found;
}

// purpose: call this for user X to notify all users who care about
// logins/logouts of user X.  returning COMPUTER_OFFLINE is a new
// addition; it adds completeness to this command, which is
// essentially asking "what is the status of user X".  returning
// OFFLINE adds consistency in that we always answer the question.

void NotifyManager::notify(Client *client, Status status) {
  NotifyIterPair p = equal_range(client->name);
  
  foreach_range (i, p.first, p.second) {
    User *lc = (*i).second;  // lc is a user that wants to know about client

    switch (status) {
    case COMING_ONLINE:
      SAY(lc, COMPUTER_ONLINE, client->name, client->linespeed);
      break;
    case GOING_OFFLINE:
      SAY(lc, COMPUTER_OFFLINE, client->name);
      break;
    }
  }
}

// purpose: to add an entry to the masterlist
// since we always add keyrefs in lowercase, we're guaranteed that
// lookups will always succeed (case doesn't matter) so long as we
// lowercase our key first.

bool NotifyManager::add(const char *notifier, Client *requester) {
  NotifyIterPair p = equal_range(notifier);
  char *newkey;

  if (p.first == p.second) { // didn't exist, allocate new
    LOG(DEBUG, "adding %s:%s (allocate)", notifier, requester->name);
    newkey = my_strdup(notifier);
  } else {
    newkey = (char *)(p.first)->first;  // grab previously allocated memory

    // key exists, see if requester was already in the map
    foreach_range (i, p.first, p.second)
      if ((*i).second == requester) {
        LOG(DEBUG, "%s:%s was already in the map!", newkey, requester->name);
        return false;
      }
    LOG(DEBUG, "adding %s:%s (reuse)", notifier, requester->name);
  }

  // add them into the mastermap and keyref list
  insert(NotifyPair(newkey, requester));
  requester->notifies.push_front(newkey);

  return true;
}

// purpose: same as add notify, but doesn't report notify_unknown or
// exists, which apparently mixes up almost every single client
// (windows client blindly adds to the notify file, linux nap client
// is just weird)

#if 0
void Server::CheckOnline(Client *requester, Command *com) {
  char *notifylist = (char *)com->data;
  char *notifier;
  
  while (get_next_argument(&notifylist, notifier, COMPNAME_MAXBUF)) {
    if (!notifies.add(notifier, requester))
      continue;

    if (User *u = get_user(notifier))
      SAY(requester, COMPUTER_ONLINE_ARGS(u));
    else
      SAY(requester, COMPUTER_OFFLINE, notifier);
  }
}

// purpose: register a chunk of notifies for a particular user.
// requester is asking to be told about users in the notifylist.  
// for each user, first add to the mastlist.  if successful, check to
// see if they're online and run the equivalent of a permissions
// check.
void Server::AddNotify(Client *requester, Command *com) {
  char *notifylist = (char *)com->data;
  char *notifier;

  while (get_next_argument(&notifylist, notifier, COMPNAME_MAXBUF)) {
    if (!notifies.add(notifier, requester)) {
      SAY(requester, NOTIFY_UNKNOWN, notifier);
      continue;
    }

   SAY(requester, NOTIFY_EXISTS, notifier);

    if (User *u = get_user(notifier))
      SAY(requester, COMPUTER_ONLINE_ARGS(u));
    else
      SAY(requester, COMPUTER_OFFLINE, notifier);
  }
}

// since the previous functionality didn't provide for an ack, neither
// will we. 
void Server::RemoveNotify(Client *requester, Command *com) {
  char *notifier, *notifylist = (char *)com->data;
  
  while (get_next_argument(&notifylist, notifier, COMPNAME_MAXBUF)) {
    if (!notifies.remove(notifier, requester))
      LOG(DEBUG, "remove of %s:%s failed", notifier, requester->name);
  }
}
// purpose: tell a user who's in their notify list
void Server::ListNotifies(Client *caller) {
  foreach (i, caller->notifies)
    SAY(caller, NOTIFY_ENTRY, *i);
  SAY(caller, NOTIFY_LIST, caller->notifies.size());
}

// purpose: to clear out all notify entries for a user (key::user)
// this is useful for:
// 1. user logging out
// 2. user sends clear notifylist command 

void Server::ClearNotifies(Client *caller) {
  slist<const char *> copy(caller->notifies);
  foreach (i, copy)
    notifies.remove(*i, caller);
  SAY(caller, NOTIFY_CLEAR, copy.size());
}

#endif
#endif
