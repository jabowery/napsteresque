/* 
 * $Id: user.cc,v 1.1.2.2 2001/08/24 02:19:17 dbrumleve Exp $
 */

#define _NAPD_USER_CC

#include "user.hh"
#include "defines.hh"

const char *User::level_str[] = {
  "Leech", "User", "Moderator", "Administrator", "Elite",
  NULL
};

const char *User::linespeed_str[] = {
  "Unknown", "14.4k Modem", "28.8k Modem", "33.6k Modem",
  "56k Modem", "56k ISDN", "128k ISDN", "Cable", "DSL",
  "T1", "T3 (or Greater)",
  NULL
};

User::User(char *_name = NULL) {
  flags.all = 0;
  name = _name ? strdup(_name) : NULL;
  linespeed = 0;
  server = NULL;
  connect_time = Time::now();
}

User::~User() {
  if (name)
    delete[] name;
}

void User::write(Command *c) {
  LOG(ERR, "unimplemented");
  THROW;
}

void User::test() {
  LOG(INFO);

  User u("bob");
}

