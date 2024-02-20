/* 
 * $Id: thread.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_THREAD_CC

extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
}

#include "thread.hh"
#include "defines.hh"

void Thread::spawn(Function f, void *a) {
  if (int e = pthread_create(&pth, NULL, f, a))
    throw e;
}
  
void *Thread::join() {
  void *r;
  if (int e = pthread_join(pth, &r))
    throw e;
  return r;
}
  
void Thread::detach() {
  if (int e = pthread_detach(pth))
    throw e;
}

void Thread::cancel() {
  pthread_cancel(pth);
}

bool operator == (const Thread &t, const Thread &u) {
  return pthread_equal(t.pth, u.pth);
}

bool operator != (const Thread &t, const Thread &u) {
  return !pthread_equal(t.pth, u.pth);
}

void Thread::test_cancel() {
  pthread_testcancel();
}
  
Thread Thread::self() {
  Thread t;
  t.pth = pthread_self();
  return t;
}

void Thread::yield() {
  sched_yield();
}

void Thread::exit() {
  int ret;
  pthread_exit(&ret);
  assert(0);
}

void Thread::kill_all() {
#ifdef LINUX
  pthread_kill_other_threads_np();
#endif
}

static void *_test_foo(void *x) {
  return (void *)((int)x + 1);
}

static void *_test_bar(void *x) {
  while (1) {
    Thread::yield();
    sleep(1);
    Thread::test_cancel();
  }
}

void Thread::test() {
  LOG(INFO);
  try {
    Thread foo(_test_foo, (void *)13);	assert((int)foo.join() == 14);
    Thread bar(_test_bar, NULL);
    bar.detach();
    bar.cancel();
  } catch (int e) {
    LOG(ERR, "%s", ERRSTR);
    assert(0);
  }
}
