/*
 * $Id: event.cc,v 1.8.20.3 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_EVENT_CC

#include <slist>

extern "C" {
#include "unistd.h"
}

#include "defines.hh"
#include "functions.hh"
#include "scheduler.hh"
#include "event.hh"

Event::Event(Handler h, Time f, const char *n = NULL) { 
  handler = h;
  frequency = f;
  name = n ? my_strdup(n) : NULL;
  last_run = 0;
}

Event::~Event() {
  if (name)
    delete[] name;
}

Time Event::elapsed() {
  return (Time::now() - last_run);
}

bool Event::ready() {
  return (elapsed() >= frequency);
}

bool Event::run() {
  last_run = Time::now();
  return handler();
}

static bool _test_handler() {
  return 1;
}

void Event::test() {
  LOG(INFO);
  Event e(_test_handler, 1, "test");	assert(!strcmp(e.name, "test"));
					assert(e.ready());
  Time t0 = Time::now();		assert(e.run());
  Time t1 = Time::now();		assert(e.last_run >= t0);
					assert(e.last_run <= t1);
					assert(!e.ready());
  --e.last_run;				assert(e.ready());
					assert(e.run());
}
