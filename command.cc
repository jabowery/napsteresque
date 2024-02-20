/*
 * $Id: command.cc,v 1.1.4.1.2.13 2001/08/23 18:30:30 dbrumleve Exp $
 */

#define _NAPD_COMMAND_CC
#warning "this class is too bloated"

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
}

#include "command.hh"
#include "defines.hh"
#include "functions.hh"
#include "user.hh"

Command::Command() {
  clear();
}

Command::Command(Code c) {
  clear();
  code = c;
}

void Command::_grow(int s) {
  if (data && s <= size)
    return;
  data = data ? (U8 *)realloc(data - 4, s + 5) + 4 : new U8[s + 5] + 4;
  data[s] = 0;
}

void Command::_free() {
  if (data)
    delete[] (data - 4);
}

Command::Command(const Command &c) {
  clear();
  size = c.size;
  code = c.code;

  if (size) {
    _grow(size + 1);
    memcpy(data, c.data, size);
    data[size] = 0;
  }
}

Command::Command(Code c, const char *str) {
  clear();
  code = c;
  size = strlen(str);
  _grow(size);
  strcpy((char *)data, str);
}

Command &Command::operator=(const Command &c) {
  _free();

  data = NULL;
  size = c.size;
  code = c.code;

  if (size) {
    _grow(size + 1);
    memcpy(data, c.data, size);
    data[size] = 0;
  }
    
  return *this;
}

Command::~Command() { 
  _free();
}

bool Command::pack(Type t0, ...) {
  va_list vl;
  va_start(vl, t0);
  bool r = vpack(t0, vl);
  va_end(vl);
  return r;
}

void Command::_push(I32 x) {
  _writer.push_back(Arg(INTEGER, sizeof(I32), (void *)x));
}

void Command::_push(const char *x) {
  _writer.push_back(Arg(STRING, strlen(x) + 1, (char *)x));
}

void Command::_push(Data x) {
  _writer.push_back(Arg(DATA, x.size, (void *)x.data));
}

void Command::_push(User *u) {
  _push(u->name);
}

void Command::_flush() {
  int s = size;

  foreach (x, _writer)
    s += sizeof(ArgHead) + x->size + 4; // 4 for good measure
  _grow(s);

  char *p = (char *)data + size;
  size = s;

  if (code < F2_CODE_START) {
    foreach (x, _writer) {
      if (p > (char *)data)
        *p++ = ' ';

      switch (x->type) {
      case INTEGER:
        p += sprintf(p, "%ld", (long)x->data);
        break;
      case STRING:
        if (strchr((char *)x->data, ' '))
          p += sprintf(p, "\"%s\"", (char *)x->data);
        else
          p += sprintf(p, "%s", (char *)x->data);
        break;
      default:
        assert(0);
      }
    }

    size = (U32)((U8*)p - data);
  } else {
    foreach (x, _writer) {
      Size s = htons(x->size);
      memcpy(p, &s, sizeof(Size));
      p += sizeof(Size);

      Type t = htons(x->type);
      memcpy(p, &t, sizeof(Type));
      p += sizeof(Type);

      switch (x->type) {
      case STRING:
      case DATA:
        memcpy(p, x->data, x->size);
        p += x->size;
        break;
      case INTEGER:
        {
          long l = htonl((long)x->data);
          memcpy(p, &l, x->size);
          p += x->size;
        }
        break;
      default: 
        assert(0);
      }
    }
  }

  _writer.clear();
}

bool Command::vpack(Type t0, va_list vl) {
  for (Type t = t0; t; t = va_arg(vl, Type)) {
    switch (t) {
    case INTEGER:
      _push(va_arg(vl, I32));
      break;
    case STRING:
      _push(va_arg(vl, char *));
      break;
    case DATA:
      _push(va_arg(vl, Data));
      break;
    default:
      assert(0);
    }
  }
 
  _flush();

  return true;
}

bool Command::unpack(...) {
  va_list vl;
  va_start(vl, this);
  bool r = vunpack(vl);
  va_end(vl);
  return r;
}

void Command::_unshift(I32 *x) {
  U8 *reader = data + offset;

  if (code < F2_CODE_START) {
    if (!*reader)
      return;
    get_next_argument((char **)&reader, (unsigned long &)*x);
    offset = reader - data;
  } else {
    Size s = ntohs(*(Size *)reader);
    if (s != sizeof(I32))
      return;

    Type t = ntohs(*(Type *)(reader + sizeof(Size)));
    if (t != Command::INTEGER)
      return;

    reader += sizeof(Size) + sizeof(Type);

    if (x) *x = ntohl(*(I32 *)reader);
    reader += s;
    offset = reader - data;
  }
}

void Command::_unshift(char **x) {
  U8 *reader = data + offset;

  if (code < F2_CODE_START) {
    if (!*reader)
      return;
    get_next_argument((char **)&reader, (char *&)*x);
    offset = reader - data;
  } else {
    Size s = ntohs(*(Size *)reader);
    Type t = ntohs(*(Type *)(reader + sizeof(Size)));

    if (t != Command::STRING)
      return;
    if (s < 1 || *(reader + sizeof(Size) + sizeof(Type) + s - 1))
      return;

    reader += sizeof(Size) + sizeof(Type);
    if (x) *x = (char *)reader;
    reader += s;
    offset = reader - data;
  }
}

void Command::_unshift(Data *x) {
  U8 *reader = data + offset;

  Size s = ntohs(*(Size *)reader);
  Type t = ntohs(*(Type *)(reader + sizeof(Size)));

  if (t != Command::DATA)
    return;

  reader += sizeof(Size) + sizeof(Type);

  if (x) {
    x->data = reader;
    x->size = s;
  }

  reader += s;
  offset = reader - data;
}

bool Command::vunpack(va_list vl) {
  offset = 0;
  while (Type u = va_arg(vl, Type)) {
    if (offset >= size)
      return false;

    switch (u) {
    case INTEGER:
      _unshift(va_arg(vl, I32 *));
      break;
    case STRING:
      _unshift(va_arg(vl, char **));
      break;
    case DATA:
      _unshift(va_arg(vl, Data *));
      break;
    default:
      return false;
    }
  }

  return true;
}

void Command::test() {
  LOG(INFO);

  I32 l23, l24;
  char *jdb;
  Data dat;

  Command c((Code)10001);

  PACK_COMMAND(&c,
    23,
    "/jdb",
    24,
    Data("null\0abc", 8)
  );

  UNPACK_COMMAND(&c,
    &l23,
    &jdb,
    &l24,
    &dat
  );				assert(c.code == 10001);
				assert(l23 == 23);
				assert(l24 == 24);
				assert(!strcmp(jdb, "/jdb"));
				assert(dat.size == 8);
				assert(dat[6] == 'b');

  char *asdf, *foo;
  int one, two, three;

  Command *d = new Command(1);
  PACK_COMMAND(d,
    "asdf",
    1,
    2,
    3,
    "foo bar"
  );				assert(!strcmp(
				  (char*)d->data,
				  "asdf 1 2 3 \"foo bar\""
				));
				assert(d->code == 1);
  *d >> asdf;			assert(!strcmp(asdf, "asdf"));
  *d >> one;			assert(one == 1);
  *d >> two;			assert(two == 2);
  *d >> three;			assert(three == 3);
  *d >> foo;			assert(!strcmp(foo, "foo bar"));

  delete d;
}

