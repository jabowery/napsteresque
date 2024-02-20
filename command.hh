/*
 * $Id: command.hh,v 1.1.4.1.2.9 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_COMMAND_HH
#define _NAPD_COMMAND_HH

#include <iostream>
#include <list>

extern "C" {
#include <unistd.h>
#include <stdarg.h>
#include <netinet/in.h>
}

#include "defines.hh"
#include "data.hh"
#include "address.hh"

class Client;
class Server;

struct Command {
  typedef U16 Size;
  typedef U16 Code;
  typedef U16 Type;

  const static Type END			= 0;
  const static Type INTEGER		= 1;
  const static Type STRING		= 2;
  const static Type DATA		= 3;

  const static U32 MAX_SIZE		= 65535;
  const static U32 F2_CODE_START	= 10000;

  Size	size;
  Code	code;
  U8*	data;

  I32 offset;

  struct ArgHead {
    Size size;
    Type type;
  };

  struct Arg : ArgHead {
    Arg(Type t, Size s, void *d) { type = t; size = s; data = d; }
    void *data;
  };

  typedef list<Arg> ArgList;
  ArgList _writer;

  Address peer;

  void _grow(int);
  void _free();

  void clear() {
    data = NULL;
    size = 0;
    code = 0;
    offset = 0;
  }

  Command();
  Command(const Command &);
  Command &operator =(const Command &);

  Command(Code);
  Command(Code, const char *);

  ~Command();

  void _flush();

  inline void _push(void) const { }
  void _push(I32);

  void _push(U32 x)		{ _push((I32)x); }
  void _push(unsigned char x)	{ _push((I32)x); }
  void _push(char x)		{ _push((I32)x); }
  void _push(unsigned short x)	{ _push((I32)x); }
  void _push(short x)		{ _push((I32)x); }
  void _push(unsigned long x)	{ _push((I32)x); }
  void _push(long x)		{ _push((I32)x); }
  void _push(const Address &x)  { _push((I32)x.ip); _push((I32)x.port); }

  void _push(const char *);
  void _push(class User *u);
  void _push(Data);

  bool pack(Type, ...);
  bool vpack(Type, va_list);

  template <class X> friend Command &operator << (Command &c, X x) {
    c._push(x);
    c._flush();
    return c;
  }

  inline void _unshift(void) const { }
  void _unshift(I32 *);
  void _unshift(U32 *x) { _unshift((I32 *)x); }
  void _unshift(char **);
  void _unshift(Data *);
  void _unshift(Address *x) {
    U32 ip, port;
    _unshift(&ip);
    _unshift(&port);
    x->ip = ip;
    x->port = port;
  }

  template <class X> friend Command &operator >> (Command &c, X &xp) {
    c._unshift(&xp);
    return c;
  }

  bool unpack(...);
  bool vunpack(va_list);

  int full() const
    { return size == offset; }

  static void test();
};

struct CommandHead {
  Command::Size size;
  Command::Code code;

  void ntoh() {
    size = ntohs(size);
    code = ntohs(code);
  }

  void hton() {
    size = htons(size);
    code = htons(code);
  }
};


// magical typed varargs by jdb

#define _FXY32(f,\
  x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xA,xB,xC,xD,xE,xF,\
  y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,yA,yB,yC,yD,yE,yF\
) \
  f(x0); f(x1); f(x2); f(x3); f(x4); f(x5); f(x6); f(x7); \
  f(x8); f(x9); f(xA); f(xB); f(xC); f(xD); f(xE); f(xF); \
  f(y0); f(y1); f(y2); f(y3); f(y4); f(y5); f(y6); f(y7); \
  f(y8); f(y9); f(yA); f(yB); f(yC); f(yD); f(yE); f(yF);

#define _PACK_COMMAND($com, \
  x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xA,xB,xC,xD,xE,xF,\
  y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,yA,yB,yC,yD,yE,yF,\
  ignore...\
) do { \
  _FXY32(($com)->_push,\
    x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xA,xB,xC,xD,xE,xF,\
    y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,yA,yB,yC,yD,yE,yF\
  ); \
  ($com)->_flush(); \
} while (0)

#define PACK_COMMAND($com, x...) \
  _PACK_COMMAND($com, x ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)

#define _UNPACK_COMMAND($com, \
  x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xA,xB,xC,xD,xE,xF,\
  y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,yA,yB,yC,yD,yE,yF,\
  ignore...\
) do { \
  ($com)->offset = 0; \
  _FXY32(($com)->_unshift,\
    x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xA,xB,xC,xD,xE,xF,\
    y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,yA,yB,yC,yD,yE,yF\
  ); \
} while (0)

#define UNPACK_COMMAND($com, x...) \
  _UNPACK_COMMAND($com, x ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,)

#define _SAY(client, code, x...) do { \
  Command *$com = new Command(code); \
  PACK_COMMAND($com ,## x); \
  (client)->write($com); \
} while (0)

// this definition looks useless but is necessary for the _ARGS macros
// defined in codes.hh to work.
#define SAY(client, code, x...) _SAY(client, code, ## x)

#define _SAY_TO(client, addr, code, x...) do { \
  Command *$com = new Command(code); \
  PACK_COMMAND($com ,## x); \
  $com->peer = (addr); \
  (client)->write($com); \
} while (0)

#define SAY_TO(client, addr, code, x...) _SAY_TO(client, addr, code, ## x)

#define _BROADCAST(remote, code, x...) do { \
  Command $com(code); PACK_COMMAND(&$com ,## x); (remote)->broadcast($com); \
} while (0)
#define BROADCAST(client, code, x...) _BROADCAST(client, code, ## x)

#endif
