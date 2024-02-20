/* 
 * $Id: file.cc,v 1.30.8.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_FILE_CC

#include <slist>
#include <list>

extern "C" {
#include <ctype.h>
}

#include "command.hh"
#include "defines.hh"
#include "functions.hh"
#include "file.hh"
#include "server.hh"

File::~File() {
  list<string> sl;
  tokenize(sl);
//  foreach (s, sl)
//    server.files.erase((*s).c_str(), this);

  if (name) delete[] name;
  if (id) delete[] id;
}

bool File::Parse(Command *com) {
  char *_name, *_id;
  UNPACK_COMMAND(com, &_name, &_id, &size, &bitrate, &frequency, &duration);

  // if the advertised size is smaller than 24 bits (3bytes), the size
  // of 1 mp3 frame, dump it (3 bytes value came from jordy)
  if (size < 3) 
    return false;

#ifdef FILENAME_PARANOIA
  if (strchr(_id, '-') == NULL)
     return false;
#endif

#if FILEID_PARANOIA > 0
  if (strlen (_id) < FILEID_MINIMUM_LEN)
    return false;
#endif

  name = my_strdup(_name);
  id = my_strdup(_id);
  return 0;
}

bool File::ParseFilename(char * name) {
  if (name) delete[] name;
  name = my_strdup(name);
  return 0;
}

bool File::isDisallowed(char *testString) {
#if 0
  for (slist<char *>::iterator i = FileDB::disallowed_extensions.begin ();
       i != FileDB::disallowed_extensions.end ();
       ++i)
    if (!strcasecmp (testString, *i))
      return true;
#endif
  return false;
}

/*
  Determines if a file name is acceptable based on occurrences of delimiters
  and fields likely to be used by rogue clients to embed disallowed valid
  extensions in the file name.

  If there is only one field, the file name passes.
  If any field matches a disallowed extension, the file name fails.

  Arguments:

  1.  The file name
  2.  A string containing the delimiter characters

  The function is destructive to the original data
*/

bool File::isNameAllowed(char *data) {
#if 0
  char *stringp = data, *first = NULL;
  int count = 0;

  // While forever ...
  while (true) {

    // Find the next token
    char *stringt = my_strsep(&stringp, FileDB::disallowed_delimiters);

    // If no token ...
    if (!stringt) {
      return true;
    }

    // If no field ...
    if (!*stringt) {
      continue;
    }

    // If not the first field ...
    if (count++) {

      // If the next field is disallowed ...
      if (isDisallowed(stringt)) return false;
      
      // If the first field was not yet evaluated ...
      if( first) {
	// If the first field is disallowed ...
	if (isDisallowed(first)) return false;

	// Indicate that the first field was evaluated
	first = NULL;
      }
    }
    // Store the location of the first field
    else first = stringt;

    // Increment the field count
    count++;
  }
#endif
   
  return true;
}

char File::GetSeparator(char *path) {
  char *pathb = path;
  char separator = '\0';
  while (*pathb) {
    if (*pathb == '/' || *pathb == '\\') {
      separator = *pathb;
      break;
    }
    
    pathb++;
  }
  
  return separator;
}

void File::tokenize(list<string> &tokens) {
  if (!name)
    return;

  // Get the length of the string - Minimum 5 (seperator + extension)
  if (strlen(name) < 5)
    return;
  
  char *pathb = my_strdup(name);
  
  char separator = GetSeparator(pathb);
  
  // Set our beginning pointer
  char *path_begin = strchr(pathb, separator);
  if (!path_begin)
    path_begin = pathb;
  
  char *path = strrchr(path_begin, separator);
  
  if (!path) 
    path = path_begin;
  else if (path != path_begin) {
    *path-- = ' ';
    
    char *tptr = strrchr(path_begin, separator);
    if (tptr)
      path = tptr;
  }
  
  if (path != path_begin)
    path++;
  
  try {
    list<string> n;
    //Lexicon::TokenizePath(path, tokens, n, false);
  } CATCH {}

  delete[] pathb;
}

char *File::info_str() {
  static char info[1024];
  info[sizeof(info) - 1] = '\0';
  snprintf(info, sizeof(info) - 1, "%s\t%s\t%lu\t%lu\t%Lu\n",
    id, name, (unsigned long)size, (unsigned long)duration, songid);
  return info;
}
