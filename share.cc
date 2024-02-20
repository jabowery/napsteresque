/* 
 * $Id: share.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_SHARE_CC

#include "server.hh"
#include "functions.hh"
#include "file.hh"
#include "filecert.hh"
#include "codes.hh"

#if 0
void Server::FileMeta(Client *caller, Command *com) {
  if (caller->capabilities.bit.filemeta2)
    log.debug("Server::FileMeta: Got FileMeta from %s", caller->name);

    unsigned long size = com->size + strlen(caller->name) + 5;
    filemeta.write(&size, sizeof(unsigned long));
    unsigned long now = htonl(NOW);
    filemeta.write(&now, sizeof(unsigned long));
    filemeta.write((char *)com->data, com->size);
    filemeta.write(caller->name, strlen(caller->name) + 1);
    filemeta.flush();
  }
}

void Server::AddSong(Client *caller, Command *com) {
  char *file_cert;
  int32_t file_size;
  time_t now = NOW;

  FileCert fcert;

  // XXX - fixme
  if (com->size < 72) {
    log.debug("Server::AddSong: Length was less than 72 bytes");
    return;
  }

  memcpy(&file_size, com->data, sizeof(int32_t));
  file_size = ntohl(file_size);
  file_cert = (char *)com->data + sizeof(int32_t);

  try {
    fcert.getFromBytes(file_cert, com->size - sizeof(int32_t));
  } CATCH {
    log.error("Server::AddSong: Invalid file certificate received from %s", caller->name);
    return;
  }

  if (!fcert.validSignature()) {
    log.debug("Server::AddSong: File certificate has invalid signature");
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  // This certificate has expired
  if ((time_t)fcert.end_date < now) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  } else if (MinCertDate && (time_t)fcert.date < (time_t)MinCertDate) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  if (BlockedSongs.find(fcert.songid) != BlockedSongs.end()) {
    log.debug("Server::AddSong: Song ID is blocked");
#ifdef RIAALOG
    time_t now = NOW;
    if (now - RIAAaddFileLogged > 10) {
      fprintf(riaalog, // +Size +Len +SongID /jdb
        "ADD_FILE\t%ld\t%ld\t%s\t%s\t%s\t%s\t%ld\t%d\t%Lu\n",
         now, caller->id(), "", "", "IB",
         "", (long)file_size, 0, fcert.songid);
      RIAAaddFileLogged = now;
    }
#endif
    NumSongBlocks++;
    return;
  }

  map<uint64_t, pair<SongCert *, time_t> >::iterator sci;

  sci = SongCertificates.find(fcert.songid);

  if (sci == SongCertificates.end()) {
    // XXX - FIXME
    caller->say(SONG_CERT, 16, fcert.md5);
    return;
  }

  // 128 album:\artist - title.mp3
  // artist - title.mp3
  SongCert *scert = (*sci).second.first;
  (*sci).second.second = now;


  TotalSongAdds++;

  char addfile_str[PACKET_MAXBUF];

  char charmd5[33];
  bin_to_hex(fcert.md5, 16, charmd5);

  sprintf(addfile_str, "\"%d %s:\\%s - %s.%s\" %s %u %u %u %u", 
          fcert.bitrate, scert->_album,
 	  scert->_artist, scert->_title,
          fcert.getFileTypeAsString(),
          charmd5, file_size, fcert.bitrate,
          fcert.frequency, 
          fcert.bitrate ?
          (file_size / 1024) / (fcert.bitrate / 8) :
          scert->head.duration);

  delete[] charmd5;
  addfile_str[sizeof(addfile_str)-1] = '\0';

  Command c(ADD_FILE, addfile_str);
  AddFile(caller, &c, fcert.songid, true);
}

void Server::AddSong2(Client *caller, Command *com) {
  char *file_cert;
  int32_t file_size, file_duration;
  time_t now = NOW;

  FileCert fcert;

  // XXX - fixme
  if (com->size < 72) {
    log.debug("Server::AddSong: Length was less than 72 bytes");
    return;
  }

  memcpy(&file_size, com->data, sizeof(int32_t));
  file_size = ntohl(file_size);

  memcpy(&file_duration, com->data + sizeof(int32_t), sizeof(int32_t));
  file_size = ntohl(file_duration);

  file_cert = (char *)com->data + sizeof(int32_t);

  try {
    fcert.getFromBytes(file_cert, com->size - sizeof(int32_t));
  } CATCH {
    log.error("Server::AddSong: Invalid file certificate received from %s", caller->name);
    return;
  }

  if (!fcert.validSignature()) {
    log.debug("Server::AddSong: File certificate has invalid signature");
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  // This certificate has expired
  if ((time_t)fcert.end_date < now) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  } else if (MinCertDate && (time_t)fcert.date < (time_t)MinCertDate) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  if (BlockedSongs.find(fcert.songid) != BlockedSongs.end()) {
    log.debug("Server::AddSong: Song ID is blocked");
#ifdef RIAALOG
    time_t now = NOW;
    if (now - RIAAaddFileLogged > 10) {
      fprintf(riaalog, // +Size +Len +SongID /jdb
        "ADD_FILE\t%ld\t%ld\t%s\t%s\t%s\t%s\t%d\t%d\t%Lu\n",
         now, caller->id(), "", "", "IB",
         "", file_size, 0, fcert.songid);
      RIAAaddFileLogged = now;
    }
#endif
    NumSongBlocks++;
    return;
  }

  map<uint64_t, pair<SongCert *, time_t> >::iterator sci;

  sci = SongCertificates.find(fcert.songid);

  if (sci == SongCertificates.end()) {
    // XXX - FIXME
    caller->say(SONG_CERT, 16, fcert.md5);
    return;
  }

  // 128 album:\artist - title.mp3
  // artist - title.mp3
  SongCert *scert = (*sci).second.first;
  (*sci).second.second = now;


  TotalSongAdds++;

  char addfile_str[PACKET_MAXBUF];
  char *charmd5;
  bin_to_hex(fcert.md5, 16, charmd5);

  sprintf(addfile_str, "\"%u %s:\\%s - %s.%s\" %s %d %u %u %u", 
          fcert.bitrate, scert->_album,
 	  scert->_artist, scert->_title,
          fcert.getFileTypeAsString(),
          charmd5, file_size, fcert.bitrate,
          fcert.frequency, file_duration);

  delete[] charmd5;
  addfile_str[sizeof(addfile_str)-1] = '\0';

  Command c(ADD_FILE, addfile_str);
  AddFile(caller, &c, fcert.songid, true);
}

void Server::RemoveSong(Client *caller, Command *com) {
  char *file_cert;
  FileCert fcert;

  // XXX - fixme
  if (com->size < 68)
    return;

  file_cert = (char *)com->data;

  try {
    fcert.getFromBytes(file_cert, com->size);
  } CATCH {
    log.error("Server::RemoveSong: Invalid file certificate received from %s",
              caller->name);
    return;
  }

  if (!fcert.validSignature())
    return;

  map<uint64_t, pair<SongCert *, time_t> >::iterator sci;

  sci = SongCertificates.find(fcert.songid);

  /* XXX - Technically this shouldn't happen, but it might. This is due to
           an oversight in the protocol. Fixme */
  if (sci == SongCertificates.end())
    return;

  SongCert *scert = (*sci).second.first;

  char removefile_str[PACKET_MAXBUF];
  sprintf(removefile_str, "\"%u %s:\\%s - %s.%s\"",
          fcert.bitrate, scert->_album,
 	  scert->_artist, scert->_title,
          fcert.getFileTypeAsString());
  removefile_str[sizeof(removefile_str)-1] = '\0';

  RemoveFile(caller, removefile_str);
}

void Server::AddSongCert(Client *caller, Command *com) {
  char *file_cert;
  char *song_cert;
  int32_t file_size;
  time_t now = NOW;

  FileCert fcert;

  // XXX - fixme
  if (com->size < 122) {
    log.debug("Server::AddSongCert: Length is less than 122");
    return;
  }

  memcpy(&file_size, com->data, sizeof(int32_t));
  file_size = ntohl(file_size);
  file_cert = (char *)com->data + sizeof(int32_t);

  try {
    fcert.getFromBytes(file_cert, com->size - sizeof(int32_t));
  } CATCH {
    log.error("Server::SongCert: Invalid file certificate received from %s", caller->name);
    return;
  }

  if (!fcert.validSignature()) {
    log.debug("Server::AddSongCert: File signature is invalid");
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  // This certificate has expired
  if ((time_t)fcert.end_date < now) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  } else if (MinCertDate && (time_t)fcert.date < (time_t)MinCertDate) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  if (BlockedSongs.find(fcert.songid) != BlockedSongs.end()) {
    log.debug("Server::AddSongCert: Song ID is blocked");
#ifdef RIAALOG
    time_t now = NOW;
    if (now - RIAAaddFileLogged > 10) {
      fprintf(riaalog, // +Size +Len +SongID /jdb
        "ADD_FILE\t%ld\t%ld\t%s\t%s\t%s\t%s\t%d\t%d\t%Lu\n",
         now, caller->id(), "", "", "IB",
         "", file_size, 0, fcert.songid);
      RIAAaddFileLogged = now;
    }
#endif
    NumSongBlocks++;
    return;
  }

  SongCert *scert;
  map<uint64_t, pair<SongCert *, time_t> >::iterator sci;

  sci = SongCertificates.find(fcert.songid);

  if (sci == SongCertificates.end()) {
     scert = new SongCert;

     // XXX - Fixme
     song_cert = (char *)com->data + sizeof(int32_t) + 68;

     try {
       scert->getFromBytes(song_cert, com->size - sizeof(int32_t) - 68);
     } CATCH {
       log.error("Server::SongCert: Invalid song certificate from %s",
                 caller->name);
       return;
     }

     if (scert->head.songid != fcert.songid) {
       log.error("Server::SongCert: Song certificate ID does not match file certificate from %s",
                 caller->name);
       delete scert;
       return;
     }

     if (!scert->validSignature()) {
       log.debug("Server::AddSongCert: Song certificate has invalid signature");
       if (caller->can(Client::CAN_IDENTIFY)) {
         cidclient->QuerySend(fcert.md5, caller->name);
         NumSongQueries++;
       }
       delete scert;
       return;
     }

     // This certificate has expired
     if ((time_t)scert->head.end_date < now) {
       if (caller->can(Client::CAN_IDENTIFY)) {
         cidclient->QuerySend(fcert.md5, caller->name);
         NumSongQueries++;
       }
       delete scert;
       return;
     } else if (MinCertDate && (time_t)scert->head.date < (time_t)MinCertDate) {
       if (caller->can(Client::CAN_IDENTIFY)) {
         cidclient->QuerySend(fcert.md5, caller->name);
         NumSongQueries++;
       }
       delete scert;
       return;
     }

     SongCertificates.insert(make_pair(scert->head.songid, 
                             make_pair(scert, now)));
  } else {
     scert = (*sci).second.first;
     (*sci).second.second = now;
  }

  TotalSongAdds++;
  char addfile_str[PACKET_MAXBUF];
  char *charmd5;
  bin_to_hex(fcert.md5, 16, charmd5);
  sprintf(addfile_str, "\"%u %s:\\%s - %s.%s\" %s %u %u %u %u", 
          fcert.bitrate, scert->_album,
 	  scert->_artist, scert->_title,
          fcert.getFileTypeAsString(),
          charmd5, file_size, fcert.bitrate,
          fcert.frequency,
          fcert.bitrate ?
          (file_size / 1024) / (fcert.bitrate / 8) :
          scert->head.duration);
  delete[] charmd5;

  addfile_str[sizeof(addfile_str)-1] = '\0';

  Command c(ADD_FILE, addfile_str);
  AddFile(caller, &c, fcert.songid, true);
}

void Server::AddSongCert2(Client *caller, Command *com) {
  char *file_cert;
  char *song_cert;
  int32_t file_size;
  int32_t file_duration;
  time_t now = NOW;

  FileCert fcert;

  // XXX - fixme
  if (com->size < 122) {
    log.debug("Server::AddSongCert: Length is less than 122");
    return;
  }

  memcpy(&file_size, com->data, sizeof(int32_t));
  file_size = ntohl(file_size);

  memcpy(&file_duration, com->data + sizeof(int32_t), sizeof(int32_t));
  file_size = ntohl(file_duration);

  file_cert = (char *)com->data + sizeof(int32_t);

  try {
    fcert.getFromBytes(file_cert, com->size - sizeof(int32_t));
  } CATCH {
    log.error("Server::SongCert: Invalid file certificate received from %s", caller->name);
    return;
  }

  if (!fcert.validSignature()) {
    log.debug("Server::AddSongCert: File signature is invalid");
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  // This certificate has expired
  if ((time_t)fcert.end_date < now) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  } else if (MinCertDate && (time_t)fcert.date < (time_t)MinCertDate) {
    if (caller->can(Client::CAN_IDENTIFY)) {
      cidclient->QuerySend(fcert.md5, caller->name);
      NumSongQueries++;
    }
    return;
  }

  if (BlockedSongs.find(fcert.songid) != BlockedSongs.end()) {
    log.debug("Server::AddSongCert: Song ID is blocked");
#ifdef RIAALOG
    time_t now = NOW;
    if (now - RIAAaddFileLogged > 10) {
      fprintf(riaalog, // +Size +Len +SongID /jdb
        "ADD_FILE\t%ld\t%ld\t%s\t%s\t%s\t%s\t%d\t%d\t%Lu\n",
         now, caller->id(), "", "", "IB",
         "", file_size, 0, fcert.songid);
      RIAAaddFileLogged = now;
    }
#endif
    NumSongBlocks++;
    return;
  }

  SongCert *scert;
  map<uint64_t, pair<SongCert *, time_t> >::iterator sci;

  sci = SongCertificates.find(fcert.songid);

  if (sci == SongCertificates.end()) {
    scert = new SongCert;

    // XXX - Fixme
    song_cert = (char *)com->data + sizeof(int32_t) + 68;

    try {
      scert->getFromBytes(song_cert, com->size - sizeof(int32_t) - 68);
    } CATCH {
      log.error("Server::SongCert: Invalid song certificate from %s", caller->name);
      return;
    }

    if (scert->head.songid != fcert.songid) {
      log.error("Server::SongCert: Song certificate ID does not match file certificate from %s", caller->name);
      delete scert;
      return;
    }

    if (!scert->validSignature()) {
      log.debug("Server::AddSongCert: Song certificate has invalid signature");
      if (caller->can(Client::CAN_IDENTIFY)) {
        cidclient->QuerySend(fcert.md5, caller->name);
        NumSongQueries++;
      }
      delete scert;
      return;
    }

    // This certificate has expired
    if ((time_t)scert->head.end_date < now) {
      if (caller->can(Client::CAN_IDENTIFY)) {
        cidclient->QuerySend(fcert.md5, caller->name);
        NumSongQueries++;
      }
      delete scert;
      return;
    } else if (MinCertDate && (time_t)scert->head.date < (time_t)MinCertDate) {
      if (caller->can(Client::CAN_IDENTIFY)) {
        cidclient->QuerySend(fcert.md5, caller->name);
        NumSongQueries++;
      }
      delete scert;
      return;
    }

    SongCertificates.insert(make_pair(scert->head.songid, make_pair(scert, now)));
  } else {
    scert = (*sci).second.first;
    (*sci).second.second = now;
  }

  TotalSongAdds++;
  char addfile_str[PACKET_MAXBUF];
  char *charmd5;
  bin_to_hex(fcert.md5, 16, charmd5);
  sprintf(addfile_str, "\"%u %s:\\%s - %s.%s\" %s %u %u %u %u", 
          fcert.bitrate, scert->_album,
 	  scert->_artist, scert->_title,
          fcert.getFileTypeAsString(),
          charmd5, file_size, fcert.bitrate,
          fcert.frequency, file_duration);
  delete[] charmd5;

  addfile_str[sizeof(addfile_str)-1] = '\0';

  Command c(ADD_FILE, addfile_str);
  AddFile(caller, &c, fcert.songid, true);
}


void Server::AddFile(Client *caller, Command *com,
 uint64_t songid = 0, bool identified = false) {

  caller->_NumFilesAttempts++;

  if (caller->_NumFiles > MaxSharedFiles)
    return;

  File *file = new File(caller);
  log.debug("FileDB::AddFile: Adding file %s", com->data);

  if (!file->Parse(com)) {
    caller->say(ERROR_MESSAGE, "invalid parameter data to add a file");
    delete file;
    return;
  }

#ifndef ALL_FILES_SHAREABLE
  static unsigned int SomeNumber;

  // This should randomize the starting point of our queries for this user
  if (NOW - _last_filemeta > 300)
     SomeNumber++;

  /* !!! - This should be cleaned up if we eliminate 10.0 and 10.1
     XXX - The hard coded values must go

    If the file has previously been identified, break
    If can't handle filemeta, break
    If the client client can handle filemeta floods and the counter is up, send
    If our main interval is up, send
  */
  if (!identified) {
    if (caller->capabilities.bit.filemeta2 &&
     caller->capabilities.bit.filemeta_flood &&
     (caller->NumFilesAttempts() + SomeNumber) %
      server.FilemetaFloodInterval == 0) ||
     NOW - _last_filemeta > (time_t)server.FilemetaInterval)) {
      client->Say(FILEMETA, "%s", file->id);
      _last_filemeta = NOW;
    }
  
  // XXX - probably should add rate limiting
    if (client->capabilities.bit.identify) {
      U8 md5[16];
      assert(strlen(file->id) == 1<<sizeof(md5));
      hex_to_bin(file->id, md5);
      server.cidclient->QuerySend(md5, client->name);
      server.NumSongQueries++;
    }

    delete file;
    return;
  }
#endif

  file->songid = songid;
  list<string> tokens;

  file->tokenize(tokens);

#ifdef RIAALOG
  time_t now = NOW;
  if (now - RIAAaddFileLogged > 10) {
    fprintf(riaalog,
      "ADD_FILE\t%ld\t%ld\t\tNB\t%s", now, caller->id(), file->info_str());
    RIAAaddFileLogged = now;
  }
#endif

  foreach (s, tokens)
    files.insert((*s).c_str(), file);

  caller->add_file(file);
  AddFileHash(file, caller);

  num_files++;
  num_bytes += file->size;

  if (filelog)
    fprintf(filelog, "%ld %s\n", NOW, file->name);
}

// NOTE: only thing to be wary of is if they try to overflow
// FILENAME_MAXBUF by specifying a path + filename that is longer. 
void Server::AddPathBlock(Client *caller, Command *com) {
  char *path;
  char *data = (char *)com->data;

  if (!get_next_argument(&data, path, FILENAME_MAXBUF)) {
    caller->Say(ERROR_MESSAGE, "parameter is unparsable");
    return;
  }

  char *filename;
  unsigned pathlen = strlen(path);

  while (get_next_argument(&data, filename, FILENAME_MAXBUF)) 
    if (pathlen + strlen(filename) < FILENAME_MAXBUF) {
      char *id;
      unsigned long size, bitrate, freq, len;

      if (!get_next_argument(&data, id, FILEID_MAXBUF) ||
	  !get_next_argument(&data, size) ||
	  !get_next_argument(&data, bitrate) ||
	  !get_next_argument(&data, freq) ||
	  !get_next_argument(&data, len)) {
	caller->Say(ERROR_MESSAGE, "parameters are unparsable");
	return;
      }

      char addfile_str[PACKET_MAXBUF] = {0};
      sprintf(addfile_str, "\"%s%s\" %s %lu %lu %lu %lu", 
	      path, filename, id, size, bitrate, freq, len);
      Command c(ADD_FILE, addfile_str);
      AddFile(caller, &c);
    }
}

// differs from ADD_FILE in that we only have to deal with the filename
void Server::RemoveFile(Client *caller, char *data) {
  char *filename;

  if (!get_next_argument(&data, filename, FILENAME_MAXBUF)) {
    caller->Say(ERROR_MESSAGE, "parameter is unparsable");
    return;
  }
  
  File *file;
  if (!(file = caller->RemoveFile(filename)))
    return;

  RemoveFileHash(file);
  TotalFiles--;
  VirtualFiles--;
  TotalSize -= file->size;
  VirtualSize -= file->size;

  delete file;
}

void Server::RemoveFile(Client *client, File *file) {
  client->RemoveFile(file);
  RemoveFileHash(file);

  TotalFiles--;
  VirtualFiles--;
  TotalSize -= file->size;
  VirtualSize -= file->size;

  delete file;
}

void Server::RemoveAllFiles(Client *caller) {
  FileList copy(caller->files);
  unsigned count = 0;
  File *file;

  foreach (i, copy) {
    if (!(file = caller->RemoveFile(*i))) { // this should never happen
      log.error("Server::RemoveAllFiles: bogus file entry %s (%lx)??",
		(*i)->name, *i);
      continue;
    }

    RemoveFileHash(file);
    TotalFiles--;
    VirtualFiles--;
    TotalSize -= file->size;
    VirtualSize -= file->size;

    delete file;

    count++;
  }

  caller->Say(REMOVE_ALL_FILES, "%d", count);
}

void Server::AddFileHash(File *file, Client *client) {
  if (!file || !client) {
    log.error("Server::AddFileHash: got a NULL file or client");
    return;
  }

  unsigned long index = Hash(file->id, FILE_HASH_TABLESIZE);
  
  FileIDTable[index].push_front(make_pair(file, client));
}

void Server::RemoveFileHash(File *file) {
  if (!file) {
    log.error("Server::RemoveFileHash: got a NULL file");
    return;
  }

  unsigned long index = Hash(file->id, FILE_HASH_TABLESIZE);

  foreach (i, FileIDTable[index])
    if ((*i).first == file) {
      FileIDTable[index].erase(i);
      return;
    }

  log.error("Server::RemoveFileHash: couldn't find %s", file->name);
}
#endif
