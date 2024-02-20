/* 
 * $Id: channel.hh,v 1.19.8.1.2.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_CHANNEL_HH
#define _NAPD_CHANNEL_HH

#include <iostream>

#include "defines.hh"
#include "client.hh"
#include "user.hh"

struct Channel {
  char *name;
  char *topic;

  UserMap members;

  Channel(const char * = NULL, const char * = NULL);
  Channel(const Channel &);
  ~Channel(void);

  void set_name(const char *_name)
    { name = _name ? strdup(_name) : NULL; }

  void set_topic(const char *_topic)
    { topic = _topic ? strdup(_topic) : NULL; }

  void insert(User *);
  void remove(User *);
  void clear();
  User *find(const char *);
  void broadcast(const Command &);

  static void test();
};

struct ChannelList : list<Channel*> {
  typedef list<Channel*>::iterator iterator;

  Channel *find(const char *name) {
    foreach (i, *this)
      if (!strcasecmp(name, (*i)->name))
        return *i;
    return NULL;
  }
};

#endif
