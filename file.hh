/*
 * $Id: file.hh,v 1.18.8.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_FILE_HH
#define _NAPD_FILE_HH

#include <set>
#include <list>
#include <string>

#include "filecert.hh"
#include "defines.hh"
#include "command.hh"
#include "ternary.hh"

class Client;
class Ternary;

struct File {
  char *name, *id;
  U32 bitrate, frequency, duration, size;
  SongID songid;
  Client *client;

  typedef enum {
    TYPE_UNKNOWN	= 0,
    TYPE_MP3		= 1,
    TYPE_WMA		= 2,
    TYPE_NAP		= 3
  } Type;

  void init() {
    name = NULL; id = NULL; client = NULL;
    bitrate = frequency = duration = size = songid = 0;
  }

  File()		{ init(); }
  File(Client *c)	{ init(); client = c; }
  ~File(void);

  bool operator==(const char *s)	{ return !strcasecmp(name, s); }
  bool operator!=(const char *s)	{ return !!strcasecmp(name, s); }

  bool Parse(Command *);
  bool ParseFilename(char *);

  void tokenize(list<string> &tokens);

  char GetSeparator(char *);
  bool isDisallowed(char *);
  bool isNameAllowed(char *);

  char *info_str();
};

typedef set<File*>		FileSet;
typedef list<File*>		FileList;
typedef list<FileSet*>		FileSetList;
typedef pair<File*, Client*>	FileHash;
typedef TernaryTree<File*>	FileIndex;

#endif
