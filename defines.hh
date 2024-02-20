/* 
 * $Id: defines.hh,v 1.53.4.2.2.4 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_DEFINES_HH
#define _NAPD_DEFINES_HH

#include <map>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include <syslog.h>
}

typedef uint64_t	U64;
typedef int64_t		I64;
typedef uint32_t	U32;
typedef int32_t		I32;
typedef uint16_t	U16;
typedef int16_t		I16;
typedef uint8_t		U8;
typedef int8_t		I8;

#define NAP_MAJOR_VERSION      "4"
#define NAP_MINOR_VERSION	"0"

#define DEFAULT_PORT		8888
#define LOAD_PORT		((DEFAULT_PORT)+1)
#define LINK_PORT               ((DEFAULT_PORT)-1)

#define DEFAULT_POLL_INTERVAL           10     // in msec 
#define DEFAULT_SCHEDULER_INTERVAL      60

//
// Paranoia levels. Higher levels of paranoia enable more exhaustive
// (and expensive) defensive checking code.
//
#ifndef PARANOIA
#define PARANOIA (1)
#endif

#ifndef FILEID_PARANOIA
#define FILEID_PARANOIA PARANOIA
#endif

// File id's are MD-5 hash values encoded as base16 (hex) strings -- thus
// valid id's should be exactly 32 characters.
#define FILEID_MINIMUM_LEN (32)

//
// Buffer Limits.
//
#define DEFAULT_MD5BLOCK_TABLESIZE 1
#define LOCAL_USER_TABLESIZE	10000
#define FILE_HASH_TABLESIZE	100000
#define POLL_ARRAY_SIZE		32768

#define FILENAME_MAXBUF         200
#define FILEID_MAXBUF           50
#define COMPNAME_MAXBUF         20
#define PACKET_MAXBUF           16384

// assorted macros, mostly stl-related
#define NOW	time(NULL)

#define ignore	catch (...) { }

#define foreach4(x, l, y, z) \
  for (typeof((l).begin()) x = (l).begin(); y; z)
#define foreach3(x, l, z)	foreach4(x, l, x != (l).end(), z)
#define foreach_erase(x, l)	foreach3(x, l, x = (l).erase(x))
#define foreach_erase_if(x, l, b) \
  foreach3(x, l, x = (b) ? (l).erase(x) : (l)+1)

#define foreach(x, l)		for (typeof((l).begin()) x = (l).begin(); x != (l).end(); ++x)

#define foreach_range(x, b, e) \
  for (typeof(b) x = (b); x != (e); ++x)

#define foreach_map(k, v, l) \
  if (typeof((l).begin()->first)  *k = NULL) 0; else \
  if (typeof((l).begin()->second) *v = NULL) 0; else \
  for (typeof((l).begin()) _pair_##k##_##v = (l).begin(); \
    k = &_pair_##k##_##v->first, v = &_pair_##k##_##v->second, \
    _pair_##k##_##v != (l).end(); ++_pair_##k##_##v)

struct ltstr {
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1, s2) < 0;
  }
};

struct ltistr {
  bool operator()(const char *s1, const char *s2) const {
    return strcasecmp(s1, s2) < 0;
  }
};

template <class T> struct strmap : map<const char *, T, ltstr> { };
template <class T> struct strimap : map<const char *, T, ltistr> { };
template <class T> struct strrel : multimap<const char *, T, ltstr> { };
template <class T> struct strirel : multimap<const char *, T, ltistr> { };

#ifdef LINUX
static inline U64 rdtsc(void) {
  U64 t;
  __asm__ __volatile__ ("rdtsc" : "=A" (t));
  return t;
}
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define htonll(x)       (x)
#define ntohll(x)       (x)
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htonll(x)       __bswap_64 (x)
#define ntohll(x)       __bswap_64 (x)
#endif

#define ERRSTR	strerror(errno)
#define CATCH	catch (int $_err_)
#define THROW	throw 1

#ifndef _NAPD_FUNCTIONS_HH
void logf(int level, const char *_pf, const char *fmt, ...);
#endif

#define LOG(level, fmtargs...) \
  ::logf(LOG_ ## level, __PRETTY_FUNCTION__ ,##fmtargs , NULL)

#endif
