/* 
 * $Id: search.hh,v 1.18.14.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_SEARCH_HH
#define _NAPD_SEARCH_HH

#include <set>
#include <list>
#include <slist>
#include <string>

extern "C" {
#include <sys/time.h>
}

#include "file.hh"
#include "defines.hh"
#include "ternary.hh"

class Client;
class RemoteClient;

struct Search {
  typedef enum {
    LINESPEED,
    BITRATE,
    FREQUENCY,
  } Field;

  string _SearchTerms;
  list<string> terms;
  char *query;

  int bitrate, bitrate_cmp;
  int frequency, frequency_cmp;
  int linespeed, linespeed_cmp;

  time_t age;
  bool is_remote;

  U32 matches;
  U32 max_matches;

  unsigned short _NumBadWords;

  void jmerge(FileSetList &, FileSetList &, Client *);

  bool LocalOnly;
  unsigned ServersSearched;

  FileList master_list;
  
  Search(unsigned, bool = false);
  ~Search(void);

  bool Load(const char *, unsigned int);  // load from rawstring
  
  bool Compare(Field, int);
  void Go(Client *, FileIndex *);

  time_t Age(void);
};

#endif
