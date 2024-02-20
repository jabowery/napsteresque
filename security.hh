/*
 * $Id: security.hh,v 1.11.8.3.2.6 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_SECURITY_HH
#define _NAPD_SECURITY_HH

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
}

struct Security {
  // this makes the stack non-executable, limiting
  // damage from buffer-overflow exploits and such
  static int stack_protect(int prot = PROT_READ | PROT_WRITE);
  static int stack_unprotect();

  // this changes the root directory to .
  static int change_root();

  // copies string into newly allocated mlocked buffer
  // and zeroes out the input string; this is a
  // paranoid approach to passwords in argv
  static char *lock_string(char *);

  // zeroes out, munlocks, and frees string returned
  // from lock_string(); not really necessary for
  // argv stuff
  static void unlock_string(char *);

  // calls setreuid() on uid
  static int change_user(uid_t);
  
  // srand with pid and tsc
  static U32 seed_random();

  // these require root
  static int set_dtable_size(I32);
  static int set_coredump_size(I32);

  static void test();
};

#endif
