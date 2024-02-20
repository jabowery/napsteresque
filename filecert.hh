/*
 * $Id: filecert.hh,v 1.5.6.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_FILECERT_HH
#define _NAPD_FILECERT_HH

#include <map>
#include <set>

extern "C" {
#include <netinet/in.h>
}

#include "defines.hh"

typedef U64 SongID;
typedef set<SongID> SongIDSet;
typedef map<SongID, pair<class SongCert *, time_t> > SongCertIndex;

class FileCert {
  const static U8 key[];

public:
  U8 version;
  U8 pad[3];
  U32 end_date;
  U32 date;
  U32 file_type;
  U32 bitrate;
  U32 frequency;
  SongID songid;
  U8 md5[16];
  U8 signature[20];

  FileCert() { memset(this, 0, sizeof(*this)); }
  ~FileCert() { }

  void ntoh() {
    end_date = ntohl(end_date);
    date = ntohl(date);
    file_type = ntohl(file_type);
    bitrate = ntohl(bitrate);
    frequency = ntohl(frequency);
    songid = ntohll(songid);
  }
    
  void hton() {
    ntoh();
  }

  void createSignature(U8 *, unsigned int *);

  void getFromBytes(const char *buffer, unsigned long bufferSize);
  bool validSignature();

  U32 getByteSize() { return sizeof(*this); }
  void copyToBytes(char *buffer, unsigned long bufferSize);

  const char *getFileTypeAsString();
};

struct SongCertHead {
  U8 version;
  U8 pad[3];
  U32 end_date;
  U32 date;
  U32 duration;
  SongID songid;

  void ntoh() {
    end_date = ntohl(end_date);
    date = ntohl(date);
    duration = ntohl(duration);
    songid = ntohll(songid);
  }
    
  void hton() {
    ntoh();
  }
};

class SongCert {
  const static U8 key[];

public:
  SongCertHead head;

  char *_artist;
  char *_title;
  char *_album;
  char *_composer;
  char *_orchestra;
  char *_soloist;
  U8 _signature[20];

protected:
   void createSignature(U8 *signature, unsigned int *signatureSize);

public:
  SongCert() { memset(this, 0, sizeof(*this)); }

  ~SongCert() {
    if (_artist)    delete[] _artist;
    if (_title)     delete[] _title;
    if (_album)     delete[] _album;
    if (_composer)  delete[] _composer;
    if (_orchestra) delete[] _orchestra;
    if (_soloist)   delete[] _soloist;
  }

  void getFromBytes(char *buffer, unsigned long bufferSize);
  void copyToBytes(char *buffer, unsigned long bufferSize);

  bool validSignature();
  U32 getByteSize();

  void set_artist(char *artist) {
    if (_artist) delete[] _artist;
    _artist = new char[strlen(artist)+1];
    strcpy(_artist, artist);
  }

  void set_title(char *title) {
    if (_title) delete[] _title;
    _title = new char[strlen(title)+1];
    strcpy(_title, title);
  }

  void set_album(char *album) {
    if (_album) delete[] _album;
    _album = new char[strlen(album)+1];
    strcpy(_album, album);
  }

  void set_composer(char *composer) {
    if (_composer) delete[] _composer;
    _composer = new char[strlen(composer)+1];
    strcpy(_composer, composer);
  }

  void set_orchestra(char *orchestra) {
    if (_orchestra) delete[] _orchestra;
    _orchestra = new char[strlen(orchestra)+1];
    strcpy(_orchestra, orchestra);
  }

  void set_soloist(char *soloist) {
    if (_soloist) delete[] _soloist;
    _soloist = new char[strlen(soloist)+1];
    strcpy(_soloist, soloist);
  }
};

#endif
