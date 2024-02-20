/* 
 * $Id: scheduled.cc,v 1.18.4.1.2.9 2001/08/23 18:29:26 dbrumleve Exp $ 
 */

#define _NAPD_SCHEDULED_CC

#include <list>
#include <algorithm>

#include "defines.hh"
#include "server.hh"
#include "scheduled.hh"
#include "filecert.hh"

#if 0
// purpose: put functions that get added to the scheduler here.  don't
//          ask why, just do it.  stop giving me lip, boy.

extern Server server;

bool UpdateUserStats(void) {
  foreach (j, server.Clients) {
    Client *c = (*j).second;
    U32 users = server.AllowVirtual ? server.VirtualUsers : server.TotalUsers;
    U32 files = server.AllowVirtual ? server.VirtualFiles : server.TotalFiles;
    unsigned long size = server.AllowVirtual ?
      ((server.VirtualSize / 1000000000) + 1) :
      ((server.TotalSize / 1000000000) + 1);
    if (c->write_queue.empty())
      SAY(c, SHARE_STATS, users, files, size, "GB", c->files.size());
  }
  return true;
}

bool UpdateServerStats(void) {
  server.BroadcastRemote(REMOTE_SHARE_STATS, "%lu %lu %Lu", 
   server.TotalUsers, server.TotalFiles, server.TotalSize);
  return true;
}

bool ReloadServers(void) {
  if (server.CheckLinking()) 
    server.ReloadRemotes(NULL);
  return true;
}

bool RandomizeServers(void) {
  random_shuffle(server.RemoteServers.begin(), server.RemoteServers.end());
  return true;
}

bool ReconnectServers(void) {
  server.InitRemotes();
  return true;
}

bool SendLinkHeartbeat(void) {
  server.BroadcastRemote(LINK_HEARTBEAT);
  return true;
}

bool ReloadBans(void) {
#if 0
  server.bans.clear();
  return server.LoadBans();
#endif
}

bool CheckIntegrity(void) {
  unsigned ucount = 0, fcount = 0;
  foreach (j, server.Clients) {
    Client *c = (*j).second;
    fcount += c->files.size();
    ucount++;
  }

  bool ret = true;

  if (ucount != server.TotalUsers) {
    log.error("CheckIntegrity: server has %d users, counted %d, adjusting", 
      server.TotalUsers, ucount);
    server.VirtualUsers += ucount - server.TotalUsers;
    server.TotalUsers = ucount;
    ret = false;
  }

  if (fcount != server.TotalFiles) {
    log.error("CheckIntegrity: server has %d files, counted %d, adjusting", 
      server.TotalFiles, fcount);
    server.VirtualFiles += fcount - server.VirtualFiles;
    server.TotalFiles = fcount;
    ret = false;
  }

  return ret;
}

bool VerifyDBConnection(void) {
  return true;
}

bool TimeoutSongCertificates(void) {
  time_t now = NOW;

  foreach (i, server.SongCertificates) {
    if (now - (*i).second.second > (time_t)server.SongCertTimeout ||
        (time_t)(*i).second.first->head.end_date < now) {
       delete (*i).second.first;
       server.SongCertificates.erase(i);
    }
  }

  return true;
}

bool FlushFileMetaLog(void) {
  char newpath[256];
  struct stat sbuf;

  // We only rename if we have written data
  stat("filemeta.log", &sbuf);

  if (sbuf.st_size) {
    server.filemeta.close();
    sprintf(newpath, "logs/filemeta.log.%ld", NOW);

    rename("filemeta.log", newpath);
    server.filemeta.open("filemeta.log", ios::out | ios::trunc | ios::binary);
  }

  return true;
}

#ifdef RIAALOG
bool RIAALogOn(void) {
  server.RIAAsearchLogged = false;
  return true;
}

bool FlushRIAALog(void) {
  char newpath[256];
  struct stat sbuf;

  stat("riaa.log", &sbuf);

  if (sbuf.st_size) {
    fclose(server.riaalog);
    sprintf(newpath, "riaa.log.%ld", NOW);
    rename("riaa.log", newpath);
    server.riaalog = fopen("riaa.log", "w");
  }

  return true;
}
#endif
#endif
