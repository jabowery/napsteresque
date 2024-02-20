/*
 * $Id: notify.hh,v 1.9.32.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_NOTIFY_HH
#define _NAPD_NOTIFY_HH

#include <slist>
#include <map>

#include "defines.hh"

class User;

typedef slist<const char*> NotifyList;
typedef strirel<User*> NotifyMap;
typedef pair<const char *, User *> NotifyPair;
typedef pair<NotifyMap::iterator, NotifyMap::iterator> NotifyIterPair;

struct NotifyManager : public NotifyMap {
  typedef enum {
    COMING_ONLINE,
    GOING_OFFLINE
  } Status;

  typedef NotifyMap::iterator iterator;

  NotifyManager(void);
  ~NotifyManager(void);
  
  bool add(const char *, Client *);
  bool remove(const char *, Client *);
  void notify(Client *, Status);
};

#endif


