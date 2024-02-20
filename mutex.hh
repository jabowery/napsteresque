/* 
 * $Id: mutex.hh,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_MUTEX_HH
#define _NAPD_MUTEX_HH

// a slightly simpler mutex.
// jdb

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
}

struct Mutex {
  pthread_mutex_t pmx;
    
  Mutex()		{ pthread_mutex_init(&pmx, NULL); }
  ~Mutex()		{ pthread_mutex_destroy(&pmx); }
  bool lock()		{ return !pthread_mutex_lock(&pmx); }
  bool lock_nb()	{ return !pthread_mutex_trylock(&pmx); }
  bool unlock()		{ return !pthread_mutex_unlock(&pmx); }

  static void test();
};

template <class X> struct Lockable : Mutex {
  X x;
  X &operator *  ()	{ return x; }
  X *operator -> ()	{ return &x; }
};

#endif
