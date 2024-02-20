/* 
 * $Id: search.cc,v 1.27.14.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_SEARCH_CC

#include "defines.hh"
#include "functions.hh"
#include "codes.hh"
#include "file.hh"
#include "search.hh"
#include "ternary.hh"
#include "client.hh"
#include "server.hh"

#if 0

extern list<Client*> SearchQueue;

Search::Search(unsigned maxres, bool is_remote = false) {
  bitrate = bitrate_cmp = 0;
  frequency = frequency_cmp = 0;
  linespeed = linespeed_cmp = 0;

  query = NULL;

  matches = 0;
  max_matches = maxres;
  age = NOW;
  is_remote = is_remote;

  ServersSearched = 0;
  LocalOnly = false;
}

Search::~Search (void) {
  terms.clear();  
  delete[] query;
}

time_t Search::Age(void) {
  return NOW - age;
}

bool largestLengthFirst(string s, string t) {
  return (s.length() > t.length());
}

bool Search::Load(const char *data, unsigned int level) {
  assert(query == NULL);
  query = my_strdup(data);

  char *ptr;
  if ((ptr = strstr(data, "MAX_RESULTS"))) {
    unsigned maxres = 0;

    RemoveField(ptr, 1);
    while (isspace(*ptr)) ptr++;
    if (*ptr) {
      maxres = atoi(ptr);
      RemoveField(ptr, 1);
      while (isspace(*ptr)) ptr++;
    }

    // if atoi succeeded and converted a usable value &&
    // maxres doesn't exceed the current search result limit
    if (maxres && (maxres < max_matches)) 
      max_matches = maxres;
  } 

  if (level >= User::ELITE && (ptr = strstr(data, "LOCAL_ONLY"))) {
    LocalOnly = true;
    RemoveField(ptr, 1);
  }

#define CONVERT_OPTION(x) \
  (!strcasecmp(x, "AT LEAST")?1:(!strcasecmp(x, "AT BEST")?-1:0))

  int x = 1;
  char field[100], op[100], value[300];
  while (GetField(data, x++, field, 100)) {
    if (!GetField(data, x++, op, 100) || 
        !GetField(data, x++, value, 300))
      return false;

    // op would be "CONTAINS", so value has the string
    if (!strcasecmp(field, "FILENAME")) {
      try {
        //Lexicon::TokenizeQuery(value, terms);
      } CATCH { }
    } else if (!strcasecmp(field, "BITRATE")) {
      bitrate_cmp = CONVERT_OPTION(op);
      bitrate = atoi(value);
    } else if (!strcasecmp(field, "FREQ")) {
      frequency_cmp = CONVERT_OPTION(op);
      frequency = atoi(value);
    } else if (!strcasecmp(field, "LINESPEED")) {
      linespeed_cmp = CONVERT_OPTION(op);
      linespeed = atoi(value);
    }
  }

  if (terms.empty())
    return false;

  terms.sort();
  terms.unique();
  terms.sort(largestLengthFirst);
  
  // do parsing here to foil an abusive user that can use the same
  // search terms but reorder them to evade the check (which used to
  // be built from the order of terms, un-uniquify'd)
  foreach (i, terms) {
    _SearchTerms += *i;
    _SearchTerms += " ";
  }
  _SearchTerms.resize(_SearchTerms.length()-1); // nuke last " "

  return true;
}

void Search::Go(Client *caller, FileIndex *tree) {
  const char *word = NULL;
  FileSetList require, exclude;
  FileSet *fs = NULL;

  foreach (t, terms) {
    word = (*t).c_str();
    if (*word != '-') {
       if ((fs = tree->find(word)) && fs->size())
         require.push_back(fs);
    } else {
       if ((fs = tree->find(word + 1)) && fs->size())
         exclude.push_back(fs);
    }
  }

  if (require.empty()) 
    return;

  jmerge(require, exclude, caller);
  matches = master_list.size();
}

bool Search::Compare(Search::Field field, int comparator) { 
  int value = 0, option = 0; 

  switch(field) {
  case LINESPEED: 
    value = linespeed;
    option = linespeed_cmp;
    break;
  case BITRATE:
    value = bitrate;
    option = bitrate_cmp;
    break;
  case FREQUENCY:
    value = frequency;
    option = frequency_cmp;
    break;
  default:
    return false;
  }

  int cmp = (comparator - value);
  switch (option) {
  case  0: return !cmp;		// should be equal
  case -1: return cmp <= 0;	// should be less
  case  1: return cmp >= 0;	// should be more
  }

  return false; // kill warnings instead of using default: 
}

/* This merging algorithm cycles through a list of sets and keeps
 * looking for a value >= curval. If it finds a run of matches equal
 * to the number of sets in the list, it will add it to the destination
 * set.
 *
 * This algorithm uses the lower_bound() method which is a variation on
 * a binary search. Since a set internally uses a binary tree, looking up
 * a value greater than or equal to another value is very quick.
 *
 * We also depend on the fact that incrementing past the end in an
 * STL list should loop us back to the beginning.
 * 
 * A near-worst case senario example:
 * 
 * l1: 01 02 03 04 05 06 07 08 09 10 11 12
 * l2: 02 04 08 10 12 14 16 18 20 22 24 26
 * l3: 03 09 12 15 18 21 24 27 30 33 36 29
 * 
 * Set curval to 0.
 * l1: Search for value >= 0: 1 (match 1)
 * l2: Search for value >= 1: 2 (match 1)
 * l3: Search for value >= 2: 3 (match 1)
 * l1: Search for value >= 3: 4 (match 1)
 * l2: Search for value >= 4: 8 (match 1)
 * l3: Search for value >= 8: 12 (match 1)
 * l1: Search for value >= 12: 12 (match 2)
 * l2: Search for value >= 12: 12 (match 3, hit)
 * l2: Set curval to next value in l2: 14 (match 1)
 * l3: Search for value >= 14: 15 (match 1)
 * l1: Search for value >= 15: end of list
 */

void Search::jmerge(FileSetList &s, FileSetList &sExclude, Client *client) {
   unsigned char matches = 0;
   unsigned int closematches = 0;
   File *curval = 0;
   FileSetList::iterator i = s.begin(), i1 = s.end();
   Client *curuser;
   int cport = client->dataport;
   int lists = s.size();

   if (lists == 1) {
     foreach (k, **i) {
         curval = *k;
         curuser = curval->client;
         if ((cport || curuser->dataport) &&
             Compare(BITRATE, curval->bitrate) &&
	     Compare(FREQUENCY, curval->frequency) &&
	     Compare(LINESPEED, curuser->linespeed) &&

//             client != curuser) {
1) {

            bool found = false;
            foreach (ii, sExclude)
              if ((*ii)->find(curval) != (*ii)->end()) { 
                found = true;
                break;
              }

            if (!found) {
               ++closematches;
               master_list.push_front(curval);

               if (matches < max_matches)
                  matches++;

               if (closematches >= max_matches) return;
            }
         }
      }
      return;
   }
  
   while (1) {
      FileSet::iterator k = (*i)->lower_bound(curval);
      if (k == (*i)->end())  
         break;

      if (*k != curval) {
         curval = *k;
         matches = 1;
      } else if (++matches == lists) {
         curuser = curval->client;
         if ((cport || curuser->dataport) &&
             Compare(BITRATE, curval->bitrate) &&
	     Compare(FREQUENCY, curval->frequency) &&
	     Compare(LINESPEED, curuser->linespeed) &&
             client != curuser) {

            bool found = false;
            foreach (ii, sExclude)
              if ((*ii)->find(curval) != (*ii)->end()) { 
                found = true;
                break;
              }


            if (!found) {
               ++closematches;
               master_list.push_front(curval);

               if (matches < max_matches)
                  matches++;

               if (closematches >= max_matches) return;
            }
         }

         if (++k == (*i)->end())
            break;
      
         curval = *k;
         matches = 1;
      }
         
      if (++i == i1) 
        i = s.begin();
   }

   return;
}

#if 0

bool Server::Search(Client *caller) {
  if (!caller->search) {
    log.error("Server::Search: %s didn't have a pending search", caller->name);
    return false;
  }

  if (caller->RemoteSearch) {
    log.error("Server::Search: %s had a RemoteSearch Active", caller->name);
    caller->RemoteSearch->ActiveSearches.erase(caller);
    caller->RemoteSearch = NULL;
  }

  struct Search *search = caller->search;
  search->Go(caller, &files);

  File *file;
  Client *lc;
  int count = 0;

  foreach (i, search->master_list) {
    file = *i;
    lc = file->client;
    
    caller->Say(SEARCH_REPLY, "\"%s\" %s %lu %d %d %d %s %lu %d %d", file->name, file->id, 
      file->size, file->bitrate, file->frequency, file->duration, lc->name, 
      lc->IP(), lc->LineSpeed(), 0); // 50

    // We can actually have more matches in our filelist than we report
    if (++count >= (int)search->matches)
       break;
  }

  if (!search->LocalOnly && LinkAllowSearches) 
    if (!ForwardSearches) {
      if (search->master_list.empty() && search->terms.size()) {
        foreach (i, RemoteServers) {
	  if ((*i)->Status() == LOGGED_ON && (*i)->HBAge() < 61) {
	    caller->RemoteSearch = *i;
	    (*i)->ActiveSearches.insert(caller);
	    (*i)->Say(REMOTE_SEARCH_REQUEST, "%s %s", caller->name, search->query);
	    
	    return false;
	  }
	}
      }
    } else {
      if (search->matches < search->max_matches) {
        foreach (i, RemoteServers) {
	  if ((*i)->Status() == LOGGED_ON && (*i)->HBAge() < 61) {
	    caller->RemoteSearch = *i;
	    (*i)->ActiveSearches.insert(caller);
	    
	    char *q = my_strdup(caller->search->query);
	    char *ptr;   
	    if ((ptr = strstr(q, "MAX_RESULTS"))) {
	      RemoveField(ptr, 1);
	      StripSpace(ptr);
	      if (*ptr) {
		RemoveField(ptr, 1);
		StripSpace(ptr);
	      }
	    }
            
	    (*i)->Say(REMOTE_SEARCH_REQUEST, "%s %s MAX_RESULTS %lu",
		      caller->name, q, search->max_matches - search->matches);
	    
	    search->ServersSearched++;
	    
	    delete[] q;
	    return false;
	  }
	}
      }
    }

  caller->Say(SEARCH_COMPLETED);
  StatsSearches++;

  delete caller->search;
  caller->search = NULL;

  return true;
}

void Server::SearchCancel(Client *caller, Command *com) {
}

void Server::SearchRequest(Client *caller, Command *com) {
  // first check for pending search.  if already in the queue, honor
  // the most recent, but maintain queue position 
  foreach (i, SearchQueue) {
    if (*i == caller) {
      log.debug("Server::SearchRequest: %s was already in the search queue", caller->name);
      
      // Remove the old search instance
      delete caller->search;
      caller->search = NULL;
      SearchQueue.erase(i);
      return;
    }
  }

  // See if we have a active remote search
  if (caller->RemoteSearch) {
    caller->RemoteSearch->ActiveSearches.erase(caller);
    caller->RemoteSearch = NULL;
  }

  if (caller->search) {
    delete caller->search;
    caller->search = NULL;
  }
  
  // create a new search
  struct Search *search = new struct Search(MaxSearchResults, false);
  if (search->Load((char *)com->data, caller->Level())) {
    caller->search = search;
    SearchQueue.push_back(caller);
  } else {
    caller->Say(SEARCH_COMPLETED);
    delete search;
  }  
}

void Server::ViewFiles(Client *caller, Command *com) {
  char *username;
  
  UNPACK_COMMAND(com, &username);
  StatsHotlistRequests++;

  Client *lc;
  RemoteClient *rc;

cerr << "username = " << username << "\n";
  if ((lc = GetLocalUser(username))) {
cerr << "lc = " << (int)lc << "\n";
    if (!lc->IsIgnoring(caller) && lc->IsVisibleTo(caller)) {
cerr << "files = " << (int)lc->files.size() << "\n";
      foreach (i, lc->files) {
	File *f = *i;

	/*
	  Removed MD5 hash from browse replies.  Old version is commented out.
	*/
	
	// caller->Say(NOTIFY_FILE, "%s \"%s\" %s %lu %d %d %d", username,
	//	    f->Name(), f->ID(), f->Size(), f->Bitrate(), f->Freq(), f->Len()); 
	
	caller->Say(NOTIFY_FILE, "%s \"%s\" 0 %lu %d %d %d",
          username, f->name, f->size, f->bitrate, f->frequency, f->duration); 
      }
      
      if (!caller->DataPort() && !lc->DataPort()) 
	caller->Say(ERROR_MODAL, "Both you and %s are firewalled; you will not be able to transfer from them.",
		    lc->name);
    }

    caller->Say(VIEW_COMPLETED, "%s %lu", username, lc->IP());

  } else if (LinkAllowBrowsing && (rc = GetRemoteUser(username))) {

    SAY(rc, REMOTE_VIEW_FILES, caller->name);

  } else {

    SAY(caller, VIEW_COMPLETED, username);

    // this has been disabled because it is likely to screw up
    // notifies in the case where we're browsing someone across a
    // remote link and LinkAllowBrowsing is turned off.
    //    caller->Say(COMPUTER_OFFLINE, "%s", username);

  }
}
#endif
#endif
