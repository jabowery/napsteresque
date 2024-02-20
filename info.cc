/* 
 * $Id: info.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_INFO_CC

#include "server.hh"

#if 0

void Server::SetClientCapabilities(Client *caller, Command *com) {
  U32 f;
  UNPACK_COMMAND(com, &f);
  caller->capabilities.all = f;
}
 
void Server::SetLineSpeed(Client *caller, Command *com) {
  U32 linespeed;
  UNPACK_COMMAND(com, &linespeed);
  caller->linespeed = linespeed;
}

void Server::SetDataPort(Client *caller, Command *com) {
  U32 port;
  UNPACK_COMMAND(com, &port);
  caller->dataport = port;
//  BroadcastRemote(REMOTE_SET_DATAPORT, "%s %lu",
//    caller->name, caller->dataport);;
}


void Server::GetLineSpeed(Client *caller, Command *com) {
  char *username;
  UNPACK_COMMAND(com, &username);

  if (User *u = get_user(username))
    SAY(caller, USER_LINESPEED_ARGS(u));
}

void Server::ShowUserInformation(Client *caller, Command *com) {
  char *username;
  UNPACK_COMMAND(com, &username);

  if (User *u = get_user(username)) {
/*
    SAY(caller, USER_INFO_ONLINE,
      u->name, User::level_str[u->level], 
      u->connect_time, "[channels]",
      u->flags.bit.muzzled ? "Muzzled" : "Active",
      u->files.size(), u->downloading, u->uploading,
      u->linespeed, u->version
    ); 
*/
  }
}

#endif
