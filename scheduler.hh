/* 
 * $Id: scheduler.hh,v 1.9.20.2 2001/08/23 18:29:26 dbrumleve Exp $
  */

/*
 * The purpose of this class is to allow us to queue non-essential 
 * shit like db updates of banlists and config information.
 * Supposedly this will work by telling the scheduler what function to
 * run, and how many minutes between each run.  It would be better if
 * we called Scheduler::run every second, but that sucks for server
 * efficiency, so we limit to minutes instead.
 */

#ifndef _NAPD_SCHEDULER_HH
#define _NAPD_SCHEDULER_HH

#include <slist>
#include "defines.hh"
#include "event.hh"

// number seconds between checks for events to run.  this means you
// can call Scheduler::Run() all you want, but it won't actually do
// anything until the interval has expired
#define DEFAULT_SCHEDULER_INTERVAL 60

struct Scheduler {
  EventList events;
  Time last_run;
  Time interval;

  Scheduler(void);
  ~Scheduler(void);
  
  // function, time per run, run on add (run first time)
  bool add(Event::Handler, Time, const char * = "", bool = true);
  bool remove(const char *);
  bool clear(void);

  U32 size(void);
  bool run(void);
};


#endif
