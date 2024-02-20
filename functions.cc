/* 
 * $Id: functions.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_FUNCTIONS_CC

#include <list>
#include <string>

extern "C" {
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>
}

#include "defines.hh"
#include "functions.hh"
#include "time.hh"

/* This is a destructive form of GetField() and must only be used if
   the
   string can be modified */
bool get_next_argument(char **src, char *&begin) {
  if (*src == NULL)
    return false;

  char *end;

  begin = *src;

  /* Basically we check to see if we have more than one argument, if
     we do we check to see if we are dealing with a quoted argument
     and if we are we adjust our pointers and try to skip over escaped
     quotes. If we aren't dealing with a string with more than one
     argument, we set the string to NULL and return begin */

  if ((end = strchr(*src, ' ')) != NULL) {
    if (**src != '"') {
      *end++ = '\0';

      while (*end == ' ')
	end++;

      *src = end;
      return true;
    } else {
      (*src)++;
      begin = *src;
      while ((*src = strchr(*src, '"')) != NULL) {
	  *(*src)++ = '\0';

	  /* Just in case we get a nonescaped quote without a space
	     after it, we don't want to advance past the first
	     character */
	  while (**src == ' ')
	    (*src)++;
	  return true;
      }
    }
  } else {
    // still need to check for quotes, but only if it begins with one
    if (**src == '"') {
      (*src)++;  // advance past it

      begin = *src;

      // instead of skipping past the data until the next quote, jump
      // to the end and work our way back, honoring the last quote if
      // any.  this seems to yield more expected results to the end
      // user than if we took the first quote in the string. 

      end = *src;
      while (*end)
	end++;
           
      end--; // retreat before the null

      // now back up
      while (end > begin && *end != '"')
	end--;

      if (end == begin) // didn't find anything, lone quote?
	return false;

      // else we stopped on a quote, terminate it
      *end = '\0';
    }

    *src = NULL;

    if (*begin == '\0')
      return false;
    else return true;
  }

  return false;
}

bool get_next_argument(char **src, char *&dst, unsigned long limit) {
  if (get_next_argument(src, dst) && strlen(dst) < limit)
    return true;

  return false;
}

bool get_next_argument(char **src, unsigned long &field) {
  char *arg = NULL;

  if (get_next_argument(src, arg)) {
    field = strtoul(arg, NULL, 10);
    return true;
  }

  return false;
}

char *my_strsep(char **stringp, const char *delim) {
  char *begin, *end;

  begin = *stringp;
  if (begin == NULL)
    return NULL;

  if (delim[0] == '\0' || delim[1] == '\0') {
    char ch = delim[0];
    
    if (ch == '\0')
      end = NULL;
    else 
      if (*begin == ch)
	end = begin;
      else if (*begin == '\0')
	end = NULL;
      else end = strchr (begin + 1, ch);
  } else end = strpbrk (begin, delim);

  if (end) {
    *end++ = '\0';
    *stringp = end;
  } else *stringp = NULL;

  return begin;
}

void __assert_fail(const char *assertion, const char *file,
                   unsigned int line, const char *function) {

  LOG(EMERG, "%s:%d %s: Assertion `%s' failed.", file, line, function,
            assertion);
  abort();
}

void __assert_perror_fail(int errnum, const char *file,
                          unsigned int line, const char *function) {

  LOG(EMERG, "%s:%d %s: Assertion failed: %s (%d)", file, line, function,
            strerror(errnum), errnum);
  abort();
}

char *crypt_password(const char *password) {
  static char crypted[17];
  ulong nr=1345345333L, add=7, nr2=0x12345671L, result[2];
  ulong tmp;

  memset(crypted, 0, sizeof(crypted));
  for (; *password ; password++) {
    if (*password == ' ' || *password == '\t')
      continue;
    tmp = (ulong) (unsigned char) *password;
    nr ^= (((nr & 63)+add)*tmp)+ (nr << 8);
    nr2 += (nr2 << 8) ^ nr;
    add += tmp;
  }

  result[0] = nr & (((ulong) 1L << 31) -1L); /* Don't use sign bit (str2int) */;
  result[1] = nr2 & (((ulong) 1L << 31) -1L);

  sprintf(crypted, "%08lx%08lx", result[0], result[1]);

  return crypted;
}

void hex_to_bin(char *in, U8 *out, U32 *out_len = NULL) {
  static char _hex_to_bin [256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,	// 0 to 9
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// A to F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// a to f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  int i;

  for (i = 0; *in; i++) {
    out[i] = _hex_to_bin[*in++] << 4;
    if (!*in) {
      i++;
      break;
    }
    out[i] |= _hex_to_bin[*in++];
  }

  if (out_len)
    *out_len = i;
}

void bin_to_hex(U8 *in, U32 in_len, char *out) {
  static char _bin_to_hex[] = "0123456789abcdef";
  U32 i;

  for (i = 0; i < in_len; i++) {
    int j = i << 1;
    out[j + 0] = _bin_to_hex[in[i] >> 4];
    out[j + 1] = _bin_to_hex[in[i] & 15];
  }
  out[i << 1] = '\0';
}

int _log_level = LOG_DEBUG;

void logf(int level, const char *_pf, const char *fmt, ...) {
  if (level > _log_level)
    return;

  char pf[64], *p = pf, *q;
  strncpy(pf, _pf, sizeof(pf) - 1);
  pf[sizeof(pf) - 1] = '\0';

  if ((q = strchr(pf, '('))) {
    *q = '\0';
    if ((q = strrchr(pf, ' ')))
      p = q + 1;
  }

  static char buf1[1024];
  static char buf2[1024];

  va_list vl;
  va_start(vl, fmt);
  vsnprintf(buf1, sizeof(buf1) - 1, fmt ? fmt : "", vl);
  va_end(vl);

  snprintf(buf2, sizeof(buf2) - 1, "%s%s%s", p, fmt ? ": " : "", buf1);
  fprintf(stderr, "%s ", (char *)Time::now());
  syslog(level, "%s", buf2);
}
