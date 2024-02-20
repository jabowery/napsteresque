/* 
 * $Id: mutex.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_MUTEX_CC

#include "thread.hh"
#include "mutex.hh"
#include "defines.hh"

void Mutex::test() {
  LOG(INFO);
  Mutex m;		assert(m.lock());
			assert(!m.lock_nb());
			assert(m.unlock());
			assert(m.lock_nb());
			assert(m.unlock());
  Lockable<int> x;	assert(x.lock());
			assert(!x.lock_nb());
  *x = 9;		assert(*x == 9);
			assert(x.unlock());
}
