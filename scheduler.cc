/*
 * $Id: scheduler.cc,v 1.8.20.3 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_SCHEDULER_CC

#include <slist>

#include "defines.hh"
#include "functions.hh"
#include "scheduler.hh"

Scheduler::Scheduler() {
  last_run = Time::now();
  interval = DEFAULT_SCHEDULER_INTERVAL;
}

Scheduler::~Scheduler() {
  clear();
  LOG(INFO, "The scheduler is exiting.");
}

bool Scheduler::add(
  Event::Handler function, Time t, 
  const char *label = "", bool runfirst = true
) {
  if (!function || !t) {
    LOG(ERR, "passed bad event \"%s\" (%lx)", label, function);
    return false;
  }

  LOG(DEBUG, "adding event \"%s\" [%d] (%lx)", label, t.secs(), function);

  Event *e = new Event(function, t, label);
  events.push_back(e);

  return true;
}

bool Scheduler::remove(const char *label) {
  foreach (i, events) {
    Event *e = *i;
    if (!strcmp(e->name, label)) {
      events.erase(i);
      return true;
    }
  }
  return false;
}

bool Scheduler::clear() {
  foreach (e, events)
    delete *e;
  events.clear();
  return true;
}

bool Scheduler::run() {
  Time now = Time::now();
  if (now - last_run < interval) 
    return true;
  else
    last_run = now;

  LOG(DEBUG, "%d seconds elapsed; %d events", interval.secs(), events.size());
  bool failed = false;
  
  foreach (i, events) {
    Event *e = *i;

    // if it's ready, it'll get run, and if the run fails
    // then we'll catch the error
    if (e->ready()) {
      if (e->run()) 
	LOG(INFO, "event \"%s\" succeeded", e->name);
      else {
	failed = true;
	LOG(ERR, "event \"%s\" failed", e->name);
      }
    }
  }

  return !failed;
}

U32 Scheduler::size() {
  return events.size();
}
