/* 
 * $Id: thread.hh,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_THREAD_HH
#define _NAPD_THREAD_HH

extern "C" {
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
}

struct Thread {
  typedef void *(*Function)(void *);

  pthread_t pth;
    
  Thread()
    { };
  Thread(pthread_t _pth)
    { pth = _pth; }
  Thread(Function f, void *a)
    { spawn(f, a); }
  ~Thread()
    { }
    
  void spawn(Function, void *);
  void *join();
  void detach();
  void cancel();
 
  friend bool operator == (const Thread &, const Thread &);
  friend bool operator != (const Thread &, const Thread &);
  
  static void test_cancel();
  static Thread self();
  static void exit();
  static void yield();
  static void kill_all();

  static void test();
};

#endif
