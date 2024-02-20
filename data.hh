/* 
 * $Id: data.hh,v 1.1.2.2 2001/08/24 02:19:17 dbrumleve Exp $
 */

#ifndef _NAPD_DATA_HH
#define _NAPD_DATA_HH

// this is just a pointer/size pair that acts like the pointer.
// it does no memory management.
// jdb

extern "C" {
#include <stdlib.h>
#include <string.h>
}

#include "defines.hh"

struct Data {
  U8 *data;
  U32 size;

  Data()			: data(NULL),	size(0)	{ }
  Data(U8 *d, U32 s)		: data(d),	size(s)	{ }
  Data(const char *d, U32 s)	: data((U8*)d),	size(s)	{ }

  Data &operator =(const Data &d)
    { data = data; size = d.size; return *this; }
  U8 &operator*()		{ return *data; }
  U8 &operator[](I32 x)		{ return data[x]; }

  Data operator +(I32 x) const	{ return Data(data + x, size - x); }

  Data &operator +=(I32 x) {
    data += x;
    size -= x;
    return *this;
  }

  operator U8 *()		{ return data; }

  bool operator ==(const Data& d) {
    return size == d.size && !memcmp(data, d.data, size);
  }
  bool operator !=(const Data& d) {
    return size != d.size || memcmp(data, d.data, size);
  }

  static void test();
};

#endif
