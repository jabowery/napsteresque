/* 
 * $Id: user.hh,v 1.1.2.2 2001/08/24 02:19:17 dbrumleve Exp $
 */

#ifndef _NAPD_USER_HH
#define _NAPD_USER_HH

#include <set>
#include <map>
#include <list>

#include "address.hh"
#include "command.hh"
#include "defines.hh"
#include "time.hh"

class Search;
class Server;

struct User {
  typedef U32 ID;
  ID id;
  char *name;

  union Flags {
    struct {
      U32 banned		: 1;
      U32 muzzled		: 1;
      U32 cloaked		: 1;
      U32 muted			: 1;
      U32 badwords		: 1;
      U32 blockchat		: 1;
      U32 blockim		: 1;
      U32 local			: 1;

#ifdef DWS
      U32 enforce_code_id	: 1;
      U32 enforce_code_update	: 1;
      U32 enforce_user_auth	: 1;
#endif
    } bit;

    U32 all;
  } flags;

  bool muzzled()	{ return flags.bit.muzzled; }
  bool muted()		{ return flags.bit.muted; }
  bool cloaked()	{ return flags.bit.cloaked; }

  typedef enum {
    LEECH,
    USER,
    MODERATOR,
    ADMINISTRATOR,
    ELITE
  } Level;
  const static char *level_str[];
  Level level;

  typedef U8 LineSpeed;
  const static char *linespeed_str[];

  Time connect_time;

  LineSpeed linespeed;
  Address address;

  Server *server;

  User(char * = NULL);
  ~User();

  void write(Command *);
  static void test();
};

typedef list<User*>	UserList;
typedef strimap<User*>	UserMap;
typedef set<User*>	UserSet;

#endif
