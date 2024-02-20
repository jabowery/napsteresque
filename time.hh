/*
 * $Id: time.hh,v 1.9.4.2.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_TIME_HH
#define _NAPD_TIME_HH

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
}

#include "defines.hh"

struct Time : timeval {
  Time()
    { tv_sec = tv_usec = 0; }
  Time(const Time& t)
    { *this = t; }
  Time(int i)
    { *this = i; }
  Time(time_t i)
    { *this = i; }
  Time(double d)
    { *this = d; }

  operator time_t() const
    { return tv_sec; }
  operator double() const
    { return tv_sec + tv_usec / 1000000.0; }

  Time &operator =(const Time& t)
    { tv_sec = t.tv_sec; tv_usec = t.tv_usec; return *this; }
  Time &operator =(int i)
    { tv_sec = i; tv_usec = 0; return *this; }
  Time &operator =(time_t i)
    { tv_sec = i; tv_usec = 0; return *this; }
  Time &operator =(double d) {
    tv_sec = (time_t)d;
    tv_usec = (time_t)((d - tv_sec) * 1000000);
    return *this;
  }

  bool operator ==(const Time& t) const
    { return tv_sec == t.tv_sec && tv_usec == t.tv_usec; }
  bool operator !=(const Time& t) const
    { return tv_sec != t.tv_sec || tv_usec != t.tv_usec; }
  bool operator < (const Time &t) const
    { return timercmp(this, &t, < ); }
  bool operator <= (const Time &t) const
    { return timercmp(this, &t, <= ); }
  bool operator > (const Time &t) const
    { return timercmp(this, &t, > ); }
  bool operator >= (const Time &t) const
    { return timercmp(this, &t, >= ); }
  bool operator !() const
    { return !tv_sec && !tv_usec; }

  Time &normalize() {
    tv_sec += tv_usec / 1000000;
    if ((tv_usec %= 1000000) < 0) {
      tv_usec += 1000000;
      tv_sec--;
    }
    return *this;
  }
 
  Time &operator +=(const Time& t)
    { tv_sec += t.tv_sec; tv_usec += t.tv_usec; return normalize(); }
  Time &operator -=(const Time& t)
    { tv_sec -= t.tv_sec; tv_usec -= t.tv_usec; return normalize(); }
  Time& operator *= (double d)
    { d *= (double)*this; return (*this = d); }
  Time& operator /= (double d)
    { d = (double)*this / d; return (*this = d); }

  Time operator +(const Time& u) const
    { Time t = *this; t += u; return t; }
  Time operator -(const Time& u) const
    { Time t = *this; t -= u; return t; }
  Time operator *(double d) const
    { Time t = *this; t *= d; return t; }
  Time operator /(double d) const
    { Time t = *this; t /= d; return t; }
  double operator /(Time t) const
    { return (double)*this / (double)t; }

  Time operator++(int post)
    { Time t = *this; ++tv_sec; return t; }
  Time &operator++()
    { ++tv_sec; return *this; }
  Time operator--(int post)
    { Time t = *this; --tv_sec; return t; }
  Time &operator--()
    { --tv_sec; return *this; }

  time_t secs() const
    { return tv_sec; }
  time_t msecs() const
    { return tv_sec * 1000 + tv_usec / 1000; }
  time_t usecs() const
    { return tv_sec * 1000000 + tv_usec; }

  operator char *();

  static Time now()
    { Time t; gettimeofday(&t, NULL); return t; }

  static void alarm(const Time &t)
    { ualarm((time_t)(t * 1000000), 0); }
  static void sleep(const Time &t)
    { usleep((time_t)(t * 1000000)); }
  static void test();
};

#endif
