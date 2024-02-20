/*
 * $Id: time.cc,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_TIME_CC
#include "time.hh"
#include "defines.hh"

const static int _time_size = 32;
const static int _buffer_size = 256 * _time_size;
static char _buffer[_buffer_size];
static int _buffer_pos = 0;

Time::operator char *() {
  struct tm *t = localtime(&tv_sec);

  char *ts = _buffer + _buffer_pos;
  strftime(ts, _time_size, "%b %d %H:%M:%S", t);
  _buffer_pos += _time_size;
  _buffer_pos %= _buffer_size;

  return ts;
}

void Time::test() {
  LOG(INFO);

  Time t0 = now();			assert(t0 > Time(1000000000));
  sleep(Time(0.1));
  Time t1 = now();			assert(Time(0.1) < t1 - t0);
  LOG(INFO, "%s", (char *)t1);
}
