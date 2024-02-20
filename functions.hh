/* 
 * $Id: functions.hh,v 1.25.4.1.2.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_FUNCTIONS_HH
#define _NAPD_FUNCTIONS_HH

#include <list>
#include <string>

#include "defines.hh"

bool get_next_argument(char **, char *&);
bool get_next_argument(char **, char *&, unsigned long);
bool get_next_argument(char **, unsigned long &);

char *my_strsep(char **, const char *);

#define my_strdup(s) \
      ({ size_t __len = strlen(s) +1;                     \
         char *__retval = new char[__len];                \
         if (__retval != NULL)                            \
           __retval = (char*)memcpy(__retval, s, __len);  \
         __retval; })

char *crypt_password(const char *);

void hex_to_bin(char *, U8 *, U32 * = NULL);
void bin_to_hex(U8 *, U32, char *);
void logf(int level, const char *_pf, const char *fmt, ...);

#ifndef _NAPD_FUNCTIONS_CC
extern int _log_level;
#endif

#endif
