/* 
 * $Id: channel.cc,v 1.42.6.1.2.12 2001/08/23 18:29:26 dbrumleve Exp $
 */ 
#define _NAPD_CHANNEL_CC

#include <map>
#include <set>
#include <list>

extern "C" {
#include <string.h>
}

#include "channel.hh"
#include "defines.hh"
#include "codes.hh"
#include "server.hh"

Channel::Channel(const char *_name = NULL, const char *_topic = NULL) {
  name = topic = NULL;
  set_name(_name);
  set_topic(_topic);
}

Channel::Channel(const Channel &c) {
  name = topic = NULL;
  set_name(c.name);
  set_topic(c.topic);
}

Channel::~Channel(void) {
  if (name)
    delete[] name;
  if (topic)
    delete[] topic;

  members.clear();
}

User *Channel::find(const char *name) {
  UserMap::iterator i = members.find(name);
  return ((i == members.end()) ? NULL : i->second);
}

void Channel::insert(User *client) {
  if (find(client->name))
    return;

  members[client->name] = client;
  SAY(client, JOIN_ACCEPTED, name);

  foreach (i, members) {
    User *m = i->second;
    SAY(client, USER_HERE_ARGS(this, m));
    if (m != client)
      SAY(m, USER_JOIN_ARGS(this, client));
  }

  SAY(client, JOIN_COMPLETED, name);
  SAY(client, CHANNEL_TOPIC, name, topic);
}

void Channel::remove(User *client) {
  foreach (i, members)
    if (i->second == client) {
      members.erase(i);
      foreach (j, members) {
        User *m = j->second;
        SAY(m, USER_PART_ARGS(this, client));
      }
      SAY(client, PART_CHAN_ARGS(this));
      return;
    }
}

void Channel::clear() {
  foreach (i, members) {
    User *m = i->second;
    SAY(m, PART_CHAN_ARGS(this));
  }
  members.clear();
}

void Channel::broadcast(const Command &com) {
  foreach (i, members) {
    User *m = i->second;
    Command x = com;
    m->write(&x);
  }
}

void Channel::test() {
  Channel channel("music", "All kinds of music");
  assert(1);
}

#ifdef WORKS
void Server::SendChannelListing(Client *caller) {
  foreach (i, channels)
    SAY(caller, CHANNEL_ENTRY_ARGS(*i));
  SAY(caller, CHANNEL_LIST);
}


void Server::JoinChannel(Client *caller, Command *com) {
  if (caller->muzzled())
    return;

  char *channelname;
  UNPACK_COMMAND(com, &channelname);
  
  // determine if the channel exists
  // if no
  //   if allownewchannels
  //     createnewchannel 
  // if yes
  //   entry vs userlevel check
  //   if channel is full and user != MODERATOR
  //     clist::derive(fullchannelname)
  // join channel

    // we found the channel, do the permissions check here instead of
    // later to avoid a potential abuse whereby a user can't join a
    // channel because of insufficient permissions, but the channel is
    // full and so a new channel gets derived, but the channel still
    // can't be joined.

    // moderators+ can join full channels.  users- get checked if the
    // channel is full and then a derivation occurs.
  bool created = false;
  Channel *channel;
  if ((channel = channels.find(channelname))) {
    if (!RequireLevel(caller, channel->RequiredLevel()))
      return;
  } else if (AllowNewChannels) {
    if (caller->Level() < MODERATOR && 
	(ContainsBadWord(channelname) || strchr(channelname, ' '))) {
      caller->Say(ERROR_MESSAGE, "invalid channel name");
      return;
    }

    channel = new Channel(channelname);
    channels.push_front(channel);
    created = true;
  } else {
    caller->Say(ERROR_MESSAGE, "channel %s does not exist", channelname);
    return;
  }

  caller->channels.push_front(channel);

  if (!channel->Add(caller)) {
    caller->RemoveChannel(channel);
    caller->Say(ERROR_MESSAGE, "channel join failed");
    return;
  }

  BroadcastRemote(REMOTE_CHANNEL_JOIN, "%s %s", caller->name, channel->name);
}

void Server::PartChannel(Client *caller, Channel *channel) {
  if (!channel->remove(caller)) 
    return;
  if (!caller->remove_channel(channel))
    return;
  BroadcastRemote(REMOTE_CHANNEL_PART, "%s %s", caller->name, channel->name);
}

void Server::PartChannel(Client *caller, Command *com) {
  char *channelname;

  UNPACK_COMMAND(com, &channelname);
  if (Channel *channel = channels.find(channelname)) {
    PartChannel(caller, channel);
  } else caller->Say(ERROR_MESSAGE, "channel part failed: channel %s does not exist", channelname);
}

void Server::SendChannelMessage(Client *caller, Command *com) {
  if (caller->muzzled()) {
    caller->Say(ERROR_MESSAGE, "cannot send to channel: you are muzzled");
    return;
  }

  char *channelname, *message;
  UNPACK_COMMAND(com, &channelname);
  if (!com->_reader || !*com->_reader)
    return;
  message = (char *)com->_reader;

  Channel *channel;
  if (!(channel = channels.find(channelname))) {
    caller->Say(ERROR_MESSAGE, "channel %s does not exist", channelname);
    return;
  }

  if (!caller->IsOn(channel)) {
    caller->Say(ERROR_MESSAGE, "you're not on channel %s", channelname);
    return;
  }

  channel->Broadcast(caller, RECV_CHAN_MESSAGE, "%s", message);
  BroadcastRemote(REMOTE_CHANNEL_MESSAGE, "%s %s \"%s\"", caller->name, channel->name, message);
  StatsChatMessages++;
}

void Server::SendChannelEmote(Client *caller, Command *com) {
  if (caller->muzzled()) {
    caller->Say(ERROR_MESSAGE, "cannot send to channel: you are muzzled");
    return;
  }

  if (caller->cloaked() && caller->Level() < ELITE) {
    caller->Say(ERROR_MESSAGE, "cannot send to channel: you are cloaked");
    return;
  }

  char *channelname, *message;
  UNPACK_COMMAND(com, &channelname, &message);
  
  Channel *channel;
  if (!(channel = channels.find(channelname))) {
    caller->Say(ERROR_MESSAGE, "channel %s does not exist", channelname);
    return;
  }

  if (!caller->IsOn(channel)) {
    caller->Say(ERROR_MESSAGE, "you're not on channel %s", channelname);
    return;
  }
  
  channel->Broadcast(caller, CHANNEL_EMOTE, "\"%s\"", message);

  StatsChatMessages++;
}

void Server::SetChannelTopic(Client *caller, Command *com) {
  if (!RequireLevel(caller, ELITE)) 
    return;
  
  char *channelname, *topic;
  UNPACK_COMMAND(com, &channelname, &topic);
  
  Channel *channel; 
  if ((channel = channels.find(channelname))) {
    channel->Topic(topic);
    Broadcast(MODERATOR, SYSTEM_MESSAGE, "%s has set the %s topic to \"%s\".", 
	      caller->name, channel->name, topic);
  } else caller->Say(ERROR_MESSAGE, "channel %s does not exist", channelname);
}

void Server::ChannelSetLevel(Client *caller, Command *com) {
  if (!RequireLevel(caller, ELITE))
    return;

  char *channelname; I32 level; 
  UNPACK_COMMAND(com, &channelname, &level);

  Channel *channel;
  if (!(channel = channels.find(channelname)))
    return;
  
  channel->required_level = level;
}

void Server::ChannelInfo(Client *caller, Command *com) {
  // assume show all channels if nothing is specified
  if (!com->size) {
    foreach (i, channels) {
      Channel *channel = *i;
      SAY(caller, CHANNEL_ENTRY2_ARGS(channel));
    }
    SAY(caller, CHANNEL_LIST2);
    return;
  }

  char *channelname;
  UNPACK_COMMAND(com, &channelname);

  Channel *channel;
  if (!(channel = channels.find(channelname)))
    return;

  SAY(caller, CHANNEL_ENTRY2_ARGS(channel));
  SAY(caller, CHANNEL_LIST2);
}

void Server::ChannelUserList(Client *caller, Command *com) {
  char *channelname;
  UNPACK_COMMAND(com, &channelname);
  
  Channel *channel;
  if (!(channel = channels.find(channelname)))
    return;

  foreach (i, channel->users)
    SAY(caller, CHANNEL_USER_ARGS(channel, *i));

  // tell the client that's the end of the list
  SAY(caller, CHANNEL_USERLIST);
}

void Server::ClearChannel(Client *caller, Command *com) {
  if (!RequireLevel(caller, ADMINISTRATOR))
    return;

  char *channelname, *reason;
  UNPACK_COMMAND(com, &channelname, &reason);

  Channel *channel = channels.find(channelname);
  if (!channel)
    return;

  Broadcast(MODERATOR, SYSTEM_MESSAGE, "%s is clearing channel %s%s%s", 
	    caller->name, channelname, (reason?": ":"."), (reason?reason:""));

  channel->Clear();
}

void Server::SendMessage(Client *caller, Command *com) {
  char *recipient, *message;

  UNPACK_COMMAND(com, &recipient);
  if (!com->_reader || !*com->_reader)
    return;
  message = (char *)com->_reader;

  if (!*message) {
    caller->Say(ERROR_MESSAGE, "no message specified");
    return;
  }

  Client *lc;
  RemoteClient *rc;

  if ((lc = GetLocalUser(recipient))) {
    if (!caller->IsVisibleTo(lc)) // if the recipient can't see me
      caller->Say(ERROR_MESSAGE, "Cannot send to %s; they cannot see you.", lc->name);
    if (!lc->IsVisibleTo(caller)) // they're invisible
      caller->Say(ERROR_MESSAGE, "user %s is not online", recipient);
    else if (caller->muzzled() && lc->Level() < MODERATOR)
      caller->Say(ERROR_MESSAGE, "cannot send message: you are muzzled");
    else if (lc->IsIgnoring(caller)) // they're ignoring me
      caller->Say(SYSTEM_MESSAGE, "User %s is ignoring you.", lc->name);
    else if (caller->IsIgnoring(lc)) // I'm ignoring them
      caller->Say(SYSTEM_MESSAGE, "Cannot send to %s; you're ignoring them.", lc->name);
    else {
      lc->Say(COMP_MESSAGE, "%s %s", caller->name, message);
      StatsInstantMessages++;
    }
    
  } else if (LinkAllowIM && 
	     (rc = GetRemoteUser(recipient))) {
    
    if (caller->muzzled() && rc->Level() < MODERATOR)
      caller->Say(ERROR_MESSAGE, "cannot send message: you are muzzled");
    else if (caller->IsIgnoring(rc))
      caller->Say(SYSTEM_MESSAGE, "Cannot send to %s; you're ignoring them.", recipient);
    else {
      rc->Say(REMOTE_COMP_MESSAGE, "%s \"%s\"", caller->name, message);
      StatsInstantMessages++;
    }
    
  } else caller->Say(ERROR_MESSAGE, "user %s is not online", recipient);
}

void Server::RemoteChannelJoin(RemoteServer *caller, Command *com) {
  char *username, *channelname;
  UNPACK_COMMAND(com, &username, &channelname);

  RemoteClient *client;
  Channel *channel;

  if (!(client = GetRemoteUser(username))) {
    log.debug("Server::RemoteChannelJoin NO CLIENT");
    return;
  }
  if (!(channel = channels.find(channelname))) {
    log.debug("Server::RemoteChannelJoin NO CHANNEL");
    return;
  }
  channel->Add(client);
}

void Server::RemoteChannelPart(RemoteServer *caller, Command *com) {
  char *username, *channelname;
  UNPACK_COMMAND(com, &username, &channelname);

  RemoteClient *client;
  Channel *channel;

  if (!(client = GetRemoteUser(username))) {
    log.debug("Server::RemoteChannelJoin NO CLIENT");
    return;
  }
  if (!(channel = channels.find(channelname))) {
    log.debug("Server::RemoteChannelJoin NO CHANNEL");
    return;
  }
  channel->Remove(client);
}

void Server::RemoteChannelMessage(RemoteServer *caller, Command *com) {
  char *username, *channelname, *message;
  UNPACK_COMMAND(com, &username, &channelname, &message);

  RemoteClient *client;
  Channel *channel;

  if (!(client = GetRemoteUser(username)))
    return;
  if (!(channel = channels.find(channelname)))
    return;

  channel->Broadcast(client, RECV_CHAN_MESSAGE, "%s", message);
}

void Server::RemoteChannelEmote(RemoteServer *caller, Command *com) {
  char *username, *channelname, *message;
  UNPACK_COMMAND(com, &username, &channelname, &message);

  RemoteClient *client;
  Channel *channel;

  if (!(client = GetRemoteUser(username)))
    return;
  if (!(channel = channels.find(channelname)))
    return;

  channel->Broadcast(client, CHANNEL_EMOTE, "%s", message);
}

#endif
