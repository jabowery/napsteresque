/* 
 * $Id: event.hh,v 1.9.20.2 2001/08/23 18:29:26 dbrumleve Exp $
  */

#ifndef _NAPD_EVENT_HH
#define _NAPD_EVENT_HH

#include <slist>
#include "defines.hh"
#include "time.hh"

struct Event {
  Time last_run;
  Time frequency;
  char *name;

  typedef bool (*Handler)(void);
  Handler handler;

  Event(Handler, Time, const char * = NULL);
  ~Event();

  bool ready();
  Time elapsed(); 
  bool run();

  static void test();
};

typedef list<Event *>	EventList;

#endif
